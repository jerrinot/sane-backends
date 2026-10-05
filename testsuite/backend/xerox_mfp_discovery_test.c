/* Net-SNMP discovery tests. Licensed under GPL + SANE exception. */
#include "sane/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "backend/xerox_mfp-discovery.h"

int sanei_debug_xerox_mfp;
void sanei_debug_xerox_mfp_call (int level, const char *message, ...)
{
  (void) level;
  (void) message;
}
void sanei_debug_ndebug (int level, const char *message, ...)
{
  (void) level;
  (void) message;
}

static int failures, checks;

static void
expect (const char *name, int condition)
{
  checks++;
  if (!condition)
    {
      fprintf (stderr, "FAIL: %s\n", name);
      failures++;
    }
}

#if HAVE_LIBSNMP && defined(HAVE_GETIFADDRS) && defined(HAVE_IFADDRS_H) \
    && defined(HAVE_NET_IF_H) && defined(HAVE_CLOCK_GETTIME)
#define NETWORK_TESTS
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <stdint.h>
#include <time.h>

static const oid scalar[] = { 1, 3, 6, 1, 4, 1, 236, 11, 5, 1, 1, 1, 26, 0 };
static const unsigned char scanner_id[] = "MFG:Samsung;MDL:M2070 Series;MODE:SCN,SPL3;";
enum reply_mode
{
  NO_REPLY, SCANNER_REPLY, WRONG_REQUEST_ID, EXPIRED_FIRST_SESSION
};
static enum reply_mode mode;
static void *server;
static int server_fd, requests, attached, sessions_opened;
static char server_endpoint[64];

static int receive_response (int, netsnmp_session *, int, netsnmp_pdu *, void *);

static int
server_callback (int operation, netsnmp_session *session, int request_id,
                 netsnmp_pdu *request, void *data)
{
  int copy;
  (void) session;
  (void) request_id;
  (void) data;
  if (operation != NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE)
    return 1;
  requests++;
  expect ("GetNext request", request->command == SNMP_MSG_GETNEXT);
  expect ("SNMPv1 request", request->version == SNMP_VERSION_1);
  expect ("public community", request->community_len == 6
          && !memcmp (request->community, "public", 6));
  expect ("DeviceID parent OID", request->variables
          && !request->variables->next_variable
          && !snmp_oid_compare (request->variables->name, request->variables->name_length,
                                scalar, OID_LENGTH (scalar) - 1));
  if (mode == NO_REPLY || mode == EXPIRED_FIRST_SESSION)
    return 1;
  for (copy = 0; copy < 2; copy++)
    {
      netsnmp_pdu *reply = snmp_clone_pdu (request);
      if (!reply)
        abort ();
      reply->command = SNMP_MSG_RESPONSE;
      reply->errstat = reply->errindex = 0;
      expect ("set response OID", !snmp_set_var_objid (reply->variables, scalar, OID_LENGTH (scalar)));
      expect ("set DeviceID", !snmp_set_var_typed_value (reply->variables, ASN_OCTET_STR,
                                                        scanner_id, sizeof (scanner_id) - 1));
      if (mode == WRONG_REQUEST_ID)
        reply->reqid++;
      if (!snmp_sess_send (server, reply))
        {
          expect ("send loopback response", 0);
          snmp_free_pdu (reply);
        }
    }
  return 1;
}

static int
client_callback (int operation, netsnmp_session *session, int request_id,
                 netsnmp_pdu *pdu, void *data)
{
  /* The test server uses an ephemeral port; emulate the printer's port 161. */
  if (operation == NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE && pdu->transport_data
      && pdu->transport_data_length == sizeof (netsnmp_indexed_addr_pair))
    {
      netsnmp_indexed_addr_pair *peer = pdu->transport_data;
      struct sockaddr_in source;
      memcpy (&source, &peer->remote_addr, sizeof (source));
      source.sin_port = htons (161);
      memcpy (&peer->remote_addr, &source, sizeof (source));
    }
  return receive_response (operation, session, request_id, pdu, data);
}

static void *
test_session_open (netsnmp_session *session)
{
  netsnmp_session redirected = *session;
  void *handle;
  struct sockaddr_in local;
  socklen_t length = sizeof (local);
  struct in_addr expected_local;
  expect ("broadcast destination", !strcmp (session->peername, "udp:127.0.0.255:161"));
  expect ("broadcast flag", session->flags & SNMP_FLAGS_UDP_BROADCAST);
  redirected.peername = server_endpoint;
  redirected.callback = client_callback;
  if (mode == EXPIRED_FIRST_SESSION)
    {
      redirected.timeout = sessions_opened ? 2000000 : 10000;
      redirected.retries = 0;
    }
  handle = snmp_sess_open (&redirected);
  expect ("open Net-SNMP client session", handle != NULL);
  if (handle)
    {
      sessions_opened++;
      inet_pton (AF_INET, session->localname, &expected_local);
      expect ("bind to interface address",
              !getsockname (snmp_sess_transport (handle)->sock,
                            (struct sockaddr *) &local, &length)
              && local.sin_addr.s_addr == expected_local.s_addr && local.sin_port);
    }
  return handle;
}

/* Add the in-process Net-SNMP responder to the backend's select loop. */
static int
test_select (int nfds, fd_set *ready, fd_set *writefds, fd_set *exceptfds,
              struct timeval *wait)
{
  int available;
  FD_SET (server_fd, ready);
  if (nfds <= server_fd)
    nfds = server_fd + 1;
  available = select (nfds, ready, writefds, exceptfds, wait);
  if (available > 0 && FD_ISSET (server_fd, ready))
    {
      snmp_sess_read (server, ready);
      FD_CLR (server_fd, ready);
      available--;
    }
  return available;
}

static int
test_getifaddrs (struct ifaddrs **result)
{
  static struct ifaddrs interfaces[4];
  static struct sockaddr_in local, second, broadcast, mask;
  memset (interfaces, 0, sizeof (interfaces));
  local.sin_family = broadcast.sin_family = mask.sin_family = AF_INET;
  inet_pton (AF_INET, "127.0.0.1", &local.sin_addr);
  second = local;
  inet_pton (AF_INET, "127.0.0.3", &second.sin_addr);
  inet_pton (AF_INET, "127.0.0.255", &broadcast.sin_addr);
  inet_pton (AF_INET, "255.255.255.0", &mask.sin_addr);
  interfaces[0].ifa_name = (char *) "inactive";
  interfaces[0].ifa_flags = IFF_UP | IFF_BROADCAST;
  interfaces[0].ifa_addr = (struct sockaddr *) &local;
  interfaces[0].ifa_broadaddr = (struct sockaddr *) &broadcast;
  interfaces[0].ifa_netmask = (struct sockaddr *) &mask;
  interfaces[1] = interfaces[0];
  interfaces[1].ifa_name = (char *) "test0";
  interfaces[1].ifa_flags |= IFF_RUNNING;
  interfaces[2] = interfaces[1];
  interfaces[2].ifa_name = (char *) "test1";
  interfaces[2].ifa_addr = (struct sockaddr *) &second;
  interfaces[3] = interfaces[1];
  interfaces[3].ifa_name = (char *) "alias";
  interfaces[0].ifa_next = &interfaces[1];
  interfaces[1].ifa_next = &interfaces[2];
  interfaces[2].ifa_next = &interfaces[3];
  *result = interfaces;
  return 0;
}

static void test_freeifaddrs (struct ifaddrs *addresses)
{
  (void) addresses;
}

#define getifaddrs test_getifaddrs
#define freeifaddrs test_freeifaddrs
#define snmp_sess_open test_session_open
#define select test_select
#endif

#include "backend/xerox_mfp-discovery.c"

#ifdef NETWORK_TESTS
#undef getifaddrs
#undef freeifaddrs
#undef snmp_sess_open
#undef select

static SANE_Status
test_attach (SANE_String_Const name)
{
  expect ("discovered numeric endpoint", !strcmp (name, "tcp 127.0.0.2 9400"));
  attached++;
  return SANE_STATUS_GOOD;
}

static void
test_responses (void)
{
  /* Response PDU captured from an M2070 using an independent SNMP client. */
  const char *hex =
    "a28194020412345678020100020100308185308182"
    "060e2b06010401816c0b050101011a000470"
    "4d46473a53616d73756e673b434d443a53504c2c5552462c4657562c5049432c4558542c535044533b"
    "4d444c3a4d32303730205365726965733b434c533a5052494e5445523b4349443a53415f53504c56335f42573b"
    "4d4f44453a53434e2c53504c332c523030303130352c5343503b";
  unsigned char packet[151];
  size_t length = sizeof (packet);
  size_t i;
  netsnmp_pdu *pdu = snmp_pdu_create (SNMP_MSG_RESPONSE);
  const char *ids[] = { "MODE:SPL3;", "MDL:SCN;MODE:NOTSCN,SCNX;", "MODE:SPL3,SCN,SCP;", "MODE:SCN" };
  unsigned int byte;
  if (!pdu)
    abort ();
  for (i = 0; i < sizeof (packet); i++)
    {
      if (sscanf (hex + i * 2, "%2x", &byte) != 1)
        abort ();
      packet[i] = byte;
    }
  pdu->version = SNMP_VERSION_1;
  pdu->community = (unsigned char *) strdup ("public");
  if (!pdu->community)
    abort ();
  pdu->community_len = 6;
  expect ("Net-SNMP parses captured PDU", !snmp_pdu_parse (pdu, packet, &length));
  expect ("captured M2070 reply accepted", scanner_response (pdu));
  for (i = 0; i < sizeof (ids) / sizeof (*ids); i++)
    {
      expect ("set test DeviceID", !snmp_set_var_typed_value (pdu->variables, ASN_OCTET_STR,
                                                            (const unsigned char *) ids[i], strlen (ids[i])));
      expect ("SCN must be a MODE token", scanner_response (pdu) == (i >= 2));
    }
  pdu->variables->type = ASN_NULL;
  expect ("non-string DeviceID rejected", !scanner_response (pdu));
  pdu->variables->type = ASN_OCTET_STR;
  pdu->errstat = SNMP_ERR_NOSUCHNAME;
  expect ("SNMP error rejected", !scanner_response (pdu));
  pdu->errstat = 0;
  pdu->errindex = 1;
  expect ("SNMP error index rejected", !scanner_response (pdu));
  pdu->errindex = 0;
  pdu->community[0] = 'x';
  expect ("wrong community rejected", !scanner_response (pdu));
  pdu->community[0] = 'p';
  pdu->version = SNMP_VERSION_2c;
  expect ("wrong SNMP version rejected", !scanner_response (pdu));
  pdu->version = SNMP_VERSION_1;
  pdu->variables->name[OID_LENGTH (scalar) - 1] = 1;
  expect ("wrong DeviceID OID rejected", !scanner_response (pdu));
  pdu->variables->name[OID_LENGTH (scalar) - 1] = 0;
  pdu->command = SNMP_MSG_GET;
  expect ("non-response PDU rejected", !scanner_response (pdu));
  pdu->command = SNMP_MSG_RESPONSE;
  {
    const struct {
      const char *name, *address;
      int port, family, accepted;
    } peers[] = {
      { "scanner on local subnet", "127.0.0.2", 161, AF_INET, 1 },
      { "wrong source port rejected", "127.0.0.2", 162, AF_INET, 0 },
      { "wrong subnet rejected", "192.0.2.1", 161, AF_INET, 0 },
      { "local address rejected", "127.0.0.1", 161, AF_INET, 0 },
      { "broadcast address rejected", "127.0.0.255", 161, AF_INET, 0 },
      { "non-IPv4 source rejected", "127.0.0.2", 161, AF_INET6, 0 }
    };
    struct discovery_results results = { 0 };
    struct discovery_interface link = { 0 };
    netsnmp_indexed_addr_pair peer = { 0 };
    struct sockaddr_in source = { 0 };
    link.results = &results;
    inet_pton (AF_INET, "127.0.0.1", &link.local);
    inet_pton (AF_INET, "127.0.0.255", &link.broadcast);
    inet_pton (AF_INET, "255.255.255.0", &link.mask);
    pdu->transport_data = &peer;
    pdu->transport_data_length = sizeof (peer);
    for (i = 0; i < sizeof (peers) / sizeof (*peers); i++)
      {
        inet_pton (AF_INET, peers[i].address, &source.sin_addr);
        source.sin_family = peers[i].family;
        source.sin_port = htons (peers[i].port);
        memcpy (&peer.remote_addr, &source, sizeof (source));
        results.count = 0;
        receive_response (NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE, NULL, 0, pdu, &link);
        expect (peers[i].name, results.count == (size_t) peers[i].accepted);
      }
    pdu->transport_data = NULL;
    pdu->transport_data_length = 0;
  }
  snmp_free_pdu (pdu);
}

static void
test_network (void)
{
  netsnmp_session session;
  netsnmp_transport *transport;
  struct sockaddr_in address;
  socklen_t length = sizeof (address);
  int64_t start, elapsed;
  snmp_sess_init (&session);
  session.version = SNMP_VERSION_1;
  session.community = (unsigned char *) "public";
  session.community_len = 6;
  session.callback = server_callback;
  transport = netsnmp_transport_open_server ("sane-discovery-test", "udp:127.0.0.2:0");
  expect ("open loopback responder", transport != NULL);
  if (!transport)
    return;
  server = snmp_sess_add (&session, transport, NULL, NULL);
  expect ("create responder session", server != NULL);
  if (!server)
    return;
  server_fd = snmp_sess_transport (server)->sock;
  if (getsockname (server_fd, (struct sockaddr *) &address, &length))
    abort ();
  snprintf (server_endpoint, sizeof (server_endpoint), "udp:127.0.0.2:%u", ntohs (address.sin_port));
  for (mode = NO_REPLY; mode <= EXPIRED_FIRST_SESSION; mode++)
    {
      requests = attached = sessions_opened = 0;
      start = now_ms ();
      expect ("network discovery status", xerox_mfp_discover (NULL, test_attach) == SANE_STATUS_GOOD);
      elapsed = now_ms () - start;
      expect ("only active unique interfaces opened", sessions_opened == 2);
      expect ("request count", requests == (mode == EXPIRED_FIRST_SESSION ? 2 : 6));
      expect ("one shared reply window", elapsed >= 900 && elapsed < 1500);
      expect ("matching responses deduplicated", attached == (mode == SCANNER_REPLY ? 1 : 0));
    }
  mode = SCANNER_REPLY;
  requests = attached = sessions_opened = 0;
  expect ("explicit interface status", xerox_mfp_discover ("test0", test_attach) == SANE_STATUS_GOOD);
  expect ("interface selector", sessions_opened == 1 && requests == 3 && attached == 1);
  requests = attached = sessions_opened = 0;
  expect ("missing interface status", xerox_mfp_discover ("missing", test_attach) == SANE_STATUS_GOOD);
  expect ("missing interface sends nothing", !sessions_opened && !requests && !attached);
  snmp_sess_close (server);
}
#endif

int main (void)
{
#ifdef NETWORK_TESTS
  test_responses ();
  test_network ();
#else
  expect ("discovery unavailable without dependencies",
          xerox_mfp_discover (NULL, NULL) == SANE_STATUS_UNSUPPORTED);
#endif
  printf ("%d checks, %d failures\n", checks, failures);
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
