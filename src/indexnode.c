/*
 * Code for interacting with indexnodes.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

 /* NI_MAX[HOST|etc]. NB: this really should be _BSD_SOURCE - the man page and
  * netdb.h say so... */
#define _POSIX_SOURCE

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <ifaddrs.h>

#include "common.h"
#include "config.h"
#include "indexnode.h"
#include "fetcher.h"


/* these are global, but they are written before any threads are spawned, and
 * thereafter only read */
static char *host, *port;
static double version = 0.0;


int indexnode_init (void)
{
    host = (char *)malloc(NI_MAXHOST * sizeof(char));
    port = (char *)malloc(NI_MAXSERV * sizeof(char));


    return !(host && port);
}

void indexnode_finalise (void)
{
    free(host);
    free(port);
}

int indexnode_find (void)
{
    /* TODO: security */
    int s4 = -1, s6 = -1;
    int s4_ok = 0, s6_ok = 0;
    int their_rc, rc = 1;
    socklen_t socklen;
    char buf[1024];
    struct sockaddr_in  sa4;
    struct sockaddr_in6 sa6;
    fd_set r_fds;
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    const int one = 1;


    if (!config_indexnode_autodetect)
    {
        strcpy(host, config_indexnode_host);
        strcpy(port, config_indexnode_port);
        version = fetcher_get_indexnode_version();

        trace_info("Static index node configured at %s:%s - version %f\n", host, port, version);

        return 0;
    }


    /* === Listen for an indexnode === */

    /* TODO: make this a config option */
    trace_info("Listening on all addresses:\n");

    errno = 0;
    if (getifaddrs(&ifaddr) == -1)
    {
        trace_warn("Cannot get interface addresses: %s\n", strerror(errno));
    }

    for (ifa = ifaddr; ifa; ifa = ifa->ifa_next)
    {
        family = ifa->ifa_addr->sa_family;

        /* For an AF_INET* interface address, display the address, interface and
         * address family */
        if (family == AF_INET || family == AF_INET6)
        {
            s = getnameinfo(ifa->ifa_addr,
                    (family == AF_INET) ? sizeof(struct sockaddr_in) :
                    sizeof(struct sockaddr_in6),
                    host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST
                );

            if (s)
            {
                trace_warn("Cannot getnameinfo(): %s\n", gai_strerror(s));
            }

            trace_info("\t%s (%s, %s)\n",
                       host,
                       ifa->ifa_name,
                       (family == AF_INET)   ? "AF_INET" :
                       (family == AF_INET6)  ? "AF_INET6" :
                       ""
                      );
        }
    }
    trace_info("\n");

    freeifaddrs(ifaddr);


    /* IPv4 */
    memset(&sa4, 0, sizeof(sa4));
    sa4.sin_family      = AF_INET;
    sa4.sin_port        = htons(42444);
    sa4.sin_addr.s_addr = INADDR_ANY;

    errno = 0;
    s4 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    setsockopt(s4, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)); /* FIXME: magic runes. Not sure if this should be used */
    if (s4 != -1)
    {
        errno = 0;
        their_rc = bind(s4, (struct sockaddr *)&sa4, sizeof(sa4));
        if (!their_rc)
        {
            s4_ok = 1;
            trace_info("Listening for index node on udp4 port %d...\n", ntohs(sa4.sin_port));
        }
        else
        {
            trace_warn("Cannot bind to udp4 indexnode listener socket: %s\n", strerror(errno));
        }
    }
    else
    {
        trace_warn("Cannot create udp4 indexnode listener socket: %s\n", strerror(errno));
    }


    /* IPv6 */
    memset(&sa6, 0, sizeof(sa6));
    sa6.sin6_family = AF_INET6;
    sa6.sin6_port   = htons(42444);
    sa6.sin6_addr   = in6addr_any;

    errno = 0;
    s6 = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    setsockopt(s6, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)); /* FIXME: magic runes. Not sure if this should be used */
    if (s6 != -1)
    {
        errno = 0;
        their_rc = bind(s6, (struct sockaddr *)&sa6, sizeof(sa6));
        if (!their_rc)
        {
            s6_ok = 1;
            trace_info("Listening for index node on udp6 port %d...\n", ntohs(sa6.sin6_port));
        }
        else
        {
            trace_warn("Cannot bind to udp6 indexnode listener socket: %s\n", strerror(errno));
        }
    }
    else
    {
        trace_warn("Cannot create udp6 indexnode listener socket: %s\n", strerror(errno));
    }


    FD_ZERO(&r_fds);
    if (s4_ok) FD_SET(s4, &r_fds);
    if (s6_ok) FD_SET(s6, &r_fds);

    errno = 0;
    their_rc = select(MAX(s4, s6) + 1, &r_fds, NULL, NULL, NULL);

    switch (their_rc)
    {
        case -1:
            trace_warn("Error waiting for indexnode broadcast: %s\n", strerror(errno));
            break;
        case 0:
            trace_warn("Not found\n");
            break;
        default:
            if (FD_ISSET(s4, &r_fds))
            {
                socklen = sizeof(sa4);
                their_rc = recvfrom(s4, buf, sizeof(buf), 0, (struct sockaddr *)&sa4, &socklen);
                if (their_rc > 0 &&
                    socklen == sizeof(sa4))
                {
                    buf[their_rc] = '\0';

                    indexnode_parse_advert_packet(buf, &version, port);

                    if (inet_ntop(AF_INET, &(sa4.sin_addr), host, NI_MAXHOST))
                    {
                        rc = 0;
                        trace_info("Found index node, version %f, at %s:%s\n", version, host, port);
                    }
                }
            }
            else if (FD_ISSET(s6, &r_fds))
            {
                socklen = sizeof(sa6);
                their_rc = recvfrom(s6, buf, sizeof(buf), 0, (struct sockaddr *)&sa6, &socklen);
                if (their_rc > 0 &&
                    socklen == sizeof(sa6))
                {
                    buf[their_rc] = '\0';

                    indexnode_parse_advert_packet(buf, &version, port);

                    if (inet_ntop(AF_INET6, &(sa6.sin6_addr), host, NI_MAXHOST))
                    {
                        rc = 0;
                        trace_info("Found index node, version %f, at %s:%s\n", version, host, port);
                    }
                }
            }
            else
            {
                assert(0);
            }
            break;
    }


    if (s4_ok) close(s4);
    if (s6_ok) close(s6);

    return rc;
}

void indexnode_parse_advert_packet (char *buf, double *version, char *port)
{
    char version_str[1024]; /* TODO: security */


    sscanf(buf, "%[^:]:%[^:]:*", version_str, port);
    *version = indexnode_parse_version(version_str);
}

double indexnode_parse_version (char *s)
{
    char version_str[1024]; /* TODO: security */


    sscanf(s, "fs2protocol-%s", version_str);
    return strtod(version_str, NULL);
}

char *indexnode_host (void)
{
    return host;
}

char *indexnode_port (void)
{
    return port;
}

double indexnode_version (void)
{
    return version;
}

/* Is the host address string we're returning an IP address? */
int indexnode_host_is_ip (void)
{
    return (strspn(host, "0123456789.") == strlen(host));
}
