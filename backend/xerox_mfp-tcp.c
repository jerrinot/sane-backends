/*
 *	SANE backend for
 *		Samsung SCX-4500W
 *
 *	Network Scanners Support
 *	Copyright 2010 Alexander Kuznetsov <acca(at)cpan.org>
 *
 * This program is licensed under GPL + SANE exception.
 * More info at http://www.sane-project.org/license.html
 *
 */

#undef	BACKEND_NAME
#define	BACKEND_NAME xerox_mfp
#define DEBUG_DECLARE_ONLY
#define DEBUG_NOT_STATIC

#include "sane/config.h"


#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#include "sane/saneopts.h"
#include "sane/sanei_scsi.h"
#include "sane/sanei_usb.h"
#include "sane/sanei_pio.h"
#include "sane/sanei_tcp.h"
#include "sane/sanei_udp.h"
#include "sane/sanei_backend.h"
#include "sane/sanei_config.h"

#include "xerox_mfp.h"
#include "xerox_mfp-discovery.h"


#define	RECV_TIMEOUT	1	/*	seconds		*/
extern int sanei_debug_xerox_mfp;

static int64_t
tcp_now_ms(void)
{
#ifdef HAVE_CLOCK_GETTIME
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) == 0)
        return (int64_t)now.tv_sec * 1000 + now.tv_nsec / 1000000;
#endif
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        return (int64_t)now.tv_sec * 1000 + now.tv_usec / 1000;
    }
}

/* Limit connect time for discovered addresses to RECV_TIMEOUT. */
static SANE_Status
tcp_open_numeric(const struct in_addr *address, int port, int *fdp)
{
    struct sockaddr_in peer;
    struct timeval wait;
    fd_set writable;
    int fd, ready, error = 0;
    int64_t deadline, remaining;
    socklen_t length = sizeof(error);

    memset(&peer, 0, sizeof(peer));
    peer.sin_family = AF_INET;
    peer.sin_addr = *address;
    peer.sin_port = htons(port);
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return SANE_STATUS_IO_ERROR;
    if (fd >= FD_SETSIZE || fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
        goto fail;
    if (connect(fd, (struct sockaddr *)&peer, sizeof(peer)) < 0) {
        if (errno != EINPROGRESS)
            goto fail;
        deadline = tcp_now_ms() + RECV_TIMEOUT * 1000;
        do {
            remaining = deadline - tcp_now_ms();
            if (remaining <= 0)
                goto fail;
            wait.tv_sec = remaining / 1000;
            wait.tv_usec = (remaining % 1000) * 1000;
            FD_ZERO(&writable);
            FD_SET(fd, &writable);
            ready = select(fd + 1, NULL, &writable, NULL, &wait);
        } while (ready < 0 && errno == EINTR);
        if (ready <= 0
            || getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &length) < 0
            || error)
            goto fail;
    }
    if (fcntl(fd, F_SETFL, 0) < 0)
        goto fail;
    *fdp = fd;
    return SANE_STATUS_GOOD;
fail:
    close(fd);
    return SANE_STATUS_IO_ERROR;
}

int	tcp_dev_request(struct device *dev,
                    SANE_Byte *cmd, size_t cmdlen,
                    SANE_Byte *resp, size_t *resplen)
{
    size_t	bytes_recv = 0;
    ssize_t	rc = 1;
    size_t	len;
    int                 inquiry = cmd && cmdlen >= 3 && cmd[2] == CMD_INQUIRY;
    int64_t             deadline = 0;


    /* Send request, if any */
    if (cmd && cmdlen) {
        len = (size_t)sanei_tcp_write(dev->dn, cmd, cmdlen);
        if (len != cmdlen) {
            DBG(1, "%s: sent only %lu bytes of %lu\n",
                __func__, (u_long)len, (u_long)cmdlen);
            return SANE_STATUS_IO_ERROR;
        }
    }

    /* Receive response, if expected */
    if (resp && resplen) {
        DBG(3, "%s: wait for %i bytes\n", __func__, (int)*resplen);
        if (inquiry)
            deadline = tcp_now_ms() + RECV_TIMEOUT * 1000;

        while (bytes_recv < *resplen && rc > 0) {
            /* SO_RCVTIMEO covers ordinary reads; inquiries need a total deadline. */
            if (inquiry) {
                fd_set readable;
                struct timeval wait;
                int64_t remaining = deadline - tcp_now_ms();
                int ready;

                if (remaining <= 0 || dev->dn < 0 || dev->dn >= FD_SETSIZE) {
                    errno = ETIMEDOUT;
                    rc = -1;
                } else {
                    wait.tv_sec = remaining / 1000;
                    wait.tv_usec = (remaining % 1000) * 1000;
                    FD_ZERO(&readable);
                    FD_SET(dev->dn, &readable);
                    ready = select(dev->dn + 1, &readable, NULL, NULL, &wait);
                    if (ready < 0 && errno == EINTR)
                        continue;
                    if (!ready)
                        errno = ETIMEDOUT;
                    rc = ready > 0 ? 1 : -1;
                }
            }
            if (rc > 0)
                rc = recv(dev->dn, resp+bytes_recv, *resplen-bytes_recv, 0);

            if (rc > 0)	bytes_recv += rc;
            else {
                DBG(1, "%s: error %s, bytes requested: %i, bytes read: %i\n",
                    __func__, strerror(errno), (int)*resplen, (int)bytes_recv);
                *resplen = bytes_recv;
                /*
                    TODO:
                	do something smarter than that!
                */
                return SANE_STATUS_GOOD;
                return SANE_STATUS_IO_ERROR;
            }
        }
    }

    *resplen = bytes_recv;

    return SANE_STATUS_GOOD;
}

SANE_Status	tcp_dev_open(struct device *dev)
{
    SANE_Status 	status;
    char		*strhost;
    char		*strport = NULL;
    const char          *service;
    int			port;
    struct		servent *sp;
    struct		timeval tv;
    SANE_String_Const	devname;
    struct in_addr      address;
    struct sockaddr_in  peer;
    socklen_t           peerlen = sizeof(peer);


    devname = dev->sane.name;
    DBG(3, "%s: open %s\n", __func__, devname);

    if (strncmp(devname, "tcp", 3) != 0)	return SANE_STATUS_INVAL;
    devname += 3;
    devname = sanei_config_skip_whitespace(devname);
    if (!*devname)	return SANE_STATUS_INVAL;

    devname = sanei_config_get_string(devname, &strhost);
    devname = sanei_config_skip_whitespace(devname);

    if (!strhost)
        return SANE_STATUS_NO_MEM;
    if (*devname) {
        devname = sanei_config_get_string(devname, &strport);
        if (!strport) {
            free(strhost);
            return SANE_STATUS_INVAL;
        }
    }
    service = strport ? strport : "9400";

    if (isdigit(*service)) {
        port = atoi(service);
    } else {
        if ((sp = getservbyname(service, "tcp"))) {
            port = ntohs(sp->s_port);
        } else {
            DBG(1, "%s: unknown TCP service %s\n", __func__, service);
            free(strhost);
            free(strport);
            return SANE_STATUS_IO_ERROR;
        }
    }

    if (port <= 0 || port > 65535)
        status = SANE_STATUS_INVAL;
    else if (inet_pton(AF_INET, strhost, &address) == 1)
        status = tcp_open_numeric(&address, port, &dev->dn);
    else
        status = sanei_tcp_open(strhost, port, &dev->dn);
    free(strhost);
    free(strport);
    if (status == SANE_STATUS_GOOD) {
        if (getpeername(dev->dn, (struct sockaddr *)&peer, &peerlen) == 0
            && peer.sin_family == AF_INET) {
            dev->tcp_address = peer.sin_addr.s_addr;
            dev->tcp_port = peer.sin_port;
        }
        tv.tv_sec  = RECV_TIMEOUT;
        tv.tv_usec = 0;
        if (setsockopt(dev->dn, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof tv) < 0) {
            DBG(1, "%s: setsockopts %s", __func__, strerror(errno));
        }
    }

    return status;
}

void
tcp_dev_close(struct device *dev)
{
    if (!dev)	return;

    DBG(3, "%s: closing dev %p\n", __func__, (void *)dev);

    /* finish all operations */
    if (dev->scanning) {
        dev->cancel = 1;
        /* flush READ_IMAGE data */
        if (dev->reading)	sane_read(dev, NULL, 1, NULL);
        /* send cancel if not sent before */
        if (dev->state != SANE_STATUS_CANCELLED)
            ret_cancel(dev, 0);
    }

    sanei_tcp_close(dev->dn);
    dev->dn = -1;
}


SANE_Status
tcp_configure_device(const char *devname, SANE_Status(*list_one)(SANE_String_Const devname))
{
    const char *next;
    char *host = NULL, *interface = NULL;
    SANE_Status status;

    next = sanei_config_get_string(devname + 3, &host);
    if (!host)
        return SANE_STATUS_NO_MEM;
    if (strcmp(host, "auto") != 0) {
        free(host);
        return list_one(devname);
    }
    free(host);
    next = sanei_config_skip_whitespace(next);
    if (*next) {
        next = sanei_config_get_string(next, &interface);
        if (!interface)
            return SANE_STATUS_INVAL;
    }
    if (interface && !*interface) {
        free(interface);
        return SANE_STATUS_INVAL;
    }
    next = sanei_config_skip_whitespace(next);
    if (*next) {
        free(interface);
        return SANE_STATUS_INVAL;
    }
    status = xerox_mfp_discover(interface, list_one);
    free(interface);
    return status;
}

/* xerox_mfp-tcp.c */
