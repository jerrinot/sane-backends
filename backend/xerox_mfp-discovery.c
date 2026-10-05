/*
 * Network discovery for Samsung-based scanners.
 * Copyright 2026 Jaromir Hamala
 * Licensed under GPL + SANE exception (see LICENSE).
 */

#include "sane/config.h"
#include "xerox_mfp-discovery.h"

#define BACKEND_NAME xerox_mfp
#define DEBUG_DECLARE_ONLY
#define DEBUG_NOT_STATIC
#include "sane/sanei_debug.h"

#if HAVE_LIBSNMP && defined(HAVE_GETIFADDRS) && defined(HAVE_IFADDRS_H) \
    && defined(HAVE_NET_IF_H) && defined(HAVE_CLOCK_GETTIME)

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#define MAX_INTERFACES 16
#define MAX_SCANNERS 32
#define DISCOVERY_MS 1000
#define ATTACH_MS 5000

/* Samsung DeviceID scalar; GetNext requests its parent OID. */
static const oid device_id[] = { 1, 3, 6, 1, 4, 1, 236, 11, 5, 1, 1, 1, 26, 0 };

struct discovery_results
{
  struct in_addr addresses[MAX_SCANNERS];
  size_t count;
};

struct discovery_interface
{
  void *session;
  struct in_addr local, broadcast;
  uint32_t mask;
  struct discovery_results *results;
};

static int64_t
now_ms (void)
{
  struct timespec now;
  if (clock_gettime (CLOCK_MONOTONIC, &now) != 0)
    return -1;
  return (int64_t) now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

static int
has_scanner (const unsigned char *id, size_t length)
{
  size_t field = 0, end, token, stop;
  /* SCN is a comma-separated token in the DeviceID's MODE field. */
  while (field < length)
    {
      for (end = field; end < length && id[end] != ';'; end++)
        ;
      if (end - field >= 5 && !memcmp (id + field, "MODE:", 5))
        {
          for (token = field + 5; token < end; token = stop + 1)
            {
              for (stop = token; stop < end && id[stop] != ','; stop++)
                ;
              if (stop - token == 3 && !memcmp (id + token, "SCN", 3))
                return 1;
            }
        }
      field = end + 1;
    }
  return 0;
}

static int
scanner_response (const netsnmp_pdu *pdu)
{
  const netsnmp_variable_list *var;
  if (pdu->command != SNMP_MSG_RESPONSE || pdu->version != SNMP_VERSION_1
      || pdu->errstat != SNMP_ERR_NOERROR || pdu->errindex
      || pdu->community_len != 6 || memcmp (pdu->community, "public", 6))
    return 0;
  for (var = pdu->variables; var; var = var->next_variable)
    if (var->type == ASN_OCTET_STR
        && snmp_oid_compare (var->name, var->name_length,
                             device_id, OID_LENGTH (device_id)) == 0
        && has_scanner (var->val.string, var->val_len))
      return 1;
  return 0;
}

static int
receive_response (int operation, netsnmp_session *session, int request_id,
                  netsnmp_pdu *pdu, void *data)
{
  struct discovery_interface *link = data;
  const netsnmp_indexed_addr_pair *peer;
  struct sockaddr_in source;
  struct in_addr address;
  size_t i;
  (void) session;
  (void) request_id;
  if (operation != NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE
      || !scanner_response (pdu)
      || !pdu->transport_data
      || pdu->transport_data_length != sizeof (*peer))
    return 0;
  peer = pdu->transport_data;
  /* Net-SNMP 5.6 stores sockaddr_in directly; later versions use a union. */
  memcpy (&source, &peer->remote_addr, sizeof (source));
  address = source.sin_addr;
  if (source.sin_family != AF_INET
      || source.sin_port != htons (161)
      || (address.s_addr & link->mask) != (link->local.s_addr & link->mask)
      || address.s_addr == link->local.s_addr
      || address.s_addr == link->broadcast.s_addr)
    return 0;
  for (i = 0; i < link->results->count; i++)
    if (link->results->addresses[i].s_addr == address.s_addr)
      break;
  if (i == link->results->count && i < MAX_SCANNERS)
    link->results->addresses[link->results->count++] = address;

  /* Returning zero keeps the request pending for other broadcast responders. */
  return 0;
}

SANE_Status
xerox_mfp_discover (const char *interface,
                    SANE_Status (*attach) (SANE_String_Const))
{
  struct discovery_interface links[MAX_INTERFACES];
  struct discovery_results results = { 0 };
  struct ifaddrs *addresses, *entry;
  size_t nlinks = 0, i;
  int64_t now, deadline;
  SANE_Status status = SANE_STATUS_GOOD;

  if (now_ms () < 0 || getifaddrs (&addresses) != 0)
    return SANE_STATUS_IO_ERROR;
  for (entry = addresses; entry && nlinks < MAX_INTERFACES; entry = entry->ifa_next)
    {
      struct discovery_interface *link = &links[nlinks];
      netsnmp_session session;
      netsnmp_transport *transport;
      netsnmp_pdu *request;
      char local[INET_ADDRSTRLEN], broadcast[INET_ADDRSTRLEN], peer[64];
      if (!entry->ifa_addr || entry->ifa_addr->sa_family != AF_INET
          || !entry->ifa_broadaddr || !entry->ifa_netmask
          || (entry->ifa_flags & (IFF_UP | IFF_RUNNING | IFF_BROADCAST))
             != (IFF_UP | IFF_RUNNING | IFF_BROADCAST)
          || (entry->ifa_flags & IFF_LOOPBACK)
          || (interface && strcmp (interface, entry->ifa_name)))
        continue;
      link->local = ((struct sockaddr_in *) entry->ifa_addr)->sin_addr;
      link->broadcast = ((struct sockaddr_in *) entry->ifa_broadaddr)->sin_addr;
      link->mask = ((struct sockaddr_in *) entry->ifa_netmask)->sin_addr.s_addr;
      link->results = &results;
      for (i = 0; i < nlinks; i++)
        if (links[i].local.s_addr == link->local.s_addr)
          break;
      if (i != nlinks
          || !inet_ntop (AF_INET, &link->local, local, sizeof (local))
          || !inet_ntop (AF_INET, &link->broadcast, broadcast, sizeof (broadcast)))
        continue;
      snprintf (peer, sizeof (peer), "udp:%s:161", broadcast);
      snmp_sess_init (&session);
      session.version = SNMP_VERSION_1;
      session.community = (unsigned char *) "public";
      session.community_len = 6;
      session.peername = peer;
      session.localname = local;
      session.flags |= SNMP_FLAGS_UDP_BROADCAST;
      session.timeout = DISCOVERY_MS * 1000 / 3;
      session.retries = 2;
      session.callback = receive_response;
      session.callback_magic = link;
      link->session = snmp_sess_open (&session);
      if (!link->session)
        {
          DBG (2, "discovery: cannot open session on %s (SNMP error %d)\n",
               entry->ifa_name, session.s_snmp_errno);
          continue;
        }
      transport = snmp_sess_transport (link->session);
      if (!transport || transport->sock < 0 || transport->sock >= FD_SETSIZE)
        {
          snmp_sess_close (link->session);
          continue;
        }
      request = snmp_pdu_create (SNMP_MSG_GETNEXT);
      if (!request || !snmp_add_null_var (request, device_id, OID_LENGTH (device_id) - 1))
        {
          snmp_free_pdu (request);
          snmp_sess_close (link->session);
          status = SANE_STATUS_NO_MEM;
          break;
        }
      if (!snmp_sess_send (link->session, request))
        {
          snmp_free_pdu (request);
          snmp_sess_close (link->session);
          DBG (2, "discovery: cannot send request on %s\n", entry->ifa_name);
          continue;
        }
      DBG (3, "discovery: querying interface %s\n", entry->ifa_name);
      nlinks++;
    }
  freeifaddrs (addresses);
  if (status != SANE_STATUS_GOOD)
    goto close_sessions;
  if (!nlinks)
    {
      DBG (1, "discovery: no usable IPv4 broadcast session\n");
      return SANE_STATUS_GOOD;
    }

  now = now_ms ();
  deadline = now + DISCOVERY_MS;
  while (now >= 0 && now < deadline)
    {
      fd_set ready;
      struct timeval wait;
      int nfds = 0, available;
      wait.tv_sec = (deadline - now) / 1000;
      wait.tv_usec = ((deadline - now) % 1000) * 1000;
      FD_ZERO (&ready);
      for (i = 0; i < nlinks; i++)
        {
          /* Each session may shorten the shared deadline, never extend it. */
          int block = 0;
          snmp_sess_select_info (links[i].session, &nfds, &ready, &wait, &block);
        }
      available = select (nfds, &ready, NULL, NULL, &wait);
      if (available < 0 && errno != EINTR)
        {
          status = SANE_STATUS_IO_ERROR;
          break;
        }
      for (i = 0; i < nlinks; i++)
        {
          if (available > 0)
            snmp_sess_read (links[i].session, &ready);
          snmp_sess_timeout (links[i].session);
        }
      now = now_ms ();
    }
  if (now < 0)
    status = SANE_STATUS_IO_ERROR;

close_sessions:
  for (i = 0; i < nlinks; i++)
    snmp_sess_close (links[i].session);
  if (status != SANE_STATUS_GOOD)
    return status;

  /* Candidate inquiries have a separate deadline from the UDP reply window. */
  now = now_ms ();
  if (now < 0)
    return SANE_STATUS_IO_ERROR;
  deadline = now + ATTACH_MS;
  for (i = 0; i < results.count; i++)
    {
      char address[INET_ADDRSTRLEN], devname[64];
      now = now_ms ();
      if (now < 0)
        return SANE_STATUS_IO_ERROR;
      if (now >= deadline)
        {
          DBG (1, "discovery: scanner validation time limit reached\n");
          break;
        }
      if (!inet_ntop (AF_INET, &results.addresses[i], address, sizeof (address)))
        continue;
      snprintf (devname, sizeof (devname), "tcp %s 9400", address);
      status = attach (devname);
      if (status == SANE_STATUS_NO_MEM)
        return status;
      if (status != SANE_STATUS_GOOD)
        DBG (2, "discovery: inquiry failed for %s (status %d)\n", address, status);
    }
  return SANE_STATUS_GOOD;
}

#else

SANE_Status
xerox_mfp_discover (const char *interface,
                    SANE_Status (*attach) (SANE_String_Const))
{
  (void) interface;
  (void) attach;
  DBG (1, "discovery: requires Net-SNMP, interface enumeration and a monotonic clock\n");
  return SANE_STATUS_UNSUPPORTED;
}

#endif
