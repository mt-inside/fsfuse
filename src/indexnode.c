/*
 * Code for interacting with indexnodes.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

 /* NB: this file relies on _POSIX_SOURCE to get NI_MAX[HOST|etc], even though
  * the man page and netdb.h seem to say it really should be _BSD_SOURCE */

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


struct _indexnode_t
{
    char *host;
    char *port;
    char *version;
};


static void parse_advert_packet (char *buf, char *port, char *version);


indexnode_t *g_indexnode;


int indexnode_init (void)
{
    return 0;
}

void indexnode_finalise (void)
{
    if (g_indexnode)
    {
        indexnode_delete(g_indexnode);
    }
}


static indexnode_t *indexnode_new (void)
{
    indexnode_t *in;


    if (g_indexnode) assert(0);


    in = malloc(sizeof(indexnode_t));
    in->host    = (char *)malloc(NI_MAXHOST * sizeof(char));
    in->port    = (char *)malloc(NI_MAXSERV * sizeof(char));
    in->version = (char *)malloc(1024       * sizeof(char));


    g_indexnode = in;


    return in;
}

void indexnode_delete (indexnode_t *in)
{
    free(in->host);
    free(in->port);
    free(in->version);

    free(in);
}


static indexnode_t *indexnode_from_config (void)
{
    indexnode_t *in = indexnode_new();


    if (in)
    {
        strcpy(in->host, config_indexnode_host);
        strcpy(in->port, config_indexnode_port);
        fetcher_get_indexnode_version(in);

        trace_info("Static index node configured at %s:%s - version %s\n",
            in->host, in->port, in->version);
    }


    return in;
}

static indexnode_t *indexnode_listen (void)
{
    /* FIXME: security */
    indexnode_t *in;
    int s4 = -1, s6 = -1;
    int s4_ok = 0, s6_ok = 0;
    int their_rc;
    socklen_t socklen;
    char buf[1024], host[NI_MAXHOST];
    struct sockaddr_in  sa4;
    struct sockaddr_in6 sa6;
    fd_set r_fds;
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    const int one = 1;


    /* TODO: make this a config option */
    trace_info("Listening on all addresses:\n");


    /* ==== Show interface addresses ==== */

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


    /* ==== Set up sockets ==== */

    /* IPv4 */
    memset(&sa4, 0, sizeof(sa4));
    sa4.sin_family      = AF_INET;
    sa4.sin_port        = htons(42444); /* TODO: config option */
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
    sa6.sin6_port   = htons(42444); /* TODO: config option */
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


    /* ==== Wait for packets ==== */

    FD_ZERO(&r_fds);
    if (s4_ok) FD_SET(s4, &r_fds);
    if (s6_ok) FD_SET(s6, &r_fds);

    /* TODO: timeout */
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
            in = indexnode_new();

            if (FD_ISSET(s4, &r_fds))
            {
                socklen = sizeof(sa4);
                their_rc = recvfrom(s4, buf, sizeof(buf), 0, (struct sockaddr *)&sa4, &socklen);
                if (their_rc > 0 &&
                    socklen == sizeof(sa4))
                {
                    buf[their_rc] = '\0';

                    parse_advert_packet(buf, in->port, in->version);

                    if (inet_ntop(AF_INET, &(sa4.sin_addr), in->host, NI_MAXHOST))
                    {
                        trace_info("Found index node, version %s, at %s:%s\n", in->version, in->host, in->port);
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

                    parse_advert_packet(buf, in->port, in->version);

                    if (inet_ntop(AF_INET6, &(sa6.sin6_addr), in->host, NI_MAXHOST))
                    {
                        trace_info("Found index node, version %s, at %s:%s\n", in->version, in->host, in->port);
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

    return in;
}

indexnode_t *indexnode_find (void)
{
    if (!config_indexnode_autodetect)
    {
        return indexnode_from_config();
    }
    else
    {
        return indexnode_listen();
    }
}

static void parse_advert_packet (char *buf, char *port, char *version)
{
    /* TODO: security */
    char s[1024];

    sscanf(buf, "%[^:]:%[^:]:*", s, port);
    indexnode_parse_version(s, version);
}

void indexnode_parse_version (char *buf, char *version)
{
    sscanf(buf, "fs2protocol-%s", version);
}

void indexnode_set_host (indexnode_t *in, char *host)
{
    in->host = strdup(host);
}

char *indexnode_get_host (indexnode_t *in)
{
    return in->host;
}

void indexnode_set_port (indexnode_t *in, char *port)
{
    in->port = strdup(port);
}

char *indexnode_get_port (indexnode_t *in)
{
    return in->port;
}

void indexnode_set_version (indexnode_t *in, char *version)
{
    in->version = strdup(version);
}

char *indexnode_get_version (indexnode_t *in)
{
    return in->version;
}
