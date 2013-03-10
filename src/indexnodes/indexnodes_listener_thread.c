/*
 * Copyright (C) 2008-2013 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Thread that listens for indexnode broadcats and raises events when they are
 * seen.
 */
#include "common.h"

#include <arpa/inet.h>
#include <errno.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "indexnodes_listener_thread.h"

#include "config.h"
#include "string_buffer.h"


static void print_network_interfaces (void);
static int get_ipv6_socket (void);
static int get_ipv4_socket (void);
static void listener_thread_event_loop (int s4, int s6, packet_received_cb_t packet_received_cb);


/* On SO_REUSEADDR: my understanding thus far is: "A socket is a 5 tuple
 * (proto, local addr, local port, remote addr, remote port).  SO_REUSEADDR just
 * says that you can reuse local addresses.  The 5 tuple still must be unique!"
 * This would seem to apply to state connected sockets - they have a remote
 * address. And it's true that SO_REUSEADDR lets you bind another socket to the
 * same local addr/port while there are connected sockets to remote addresses
 * from on that port. We're talking about state listening sockets here, which I
 * don't believe have a meaningful remote address.
 * However, even if they did, our 5-tuples here are still identical - the IPv4
 * and IPv6 sockets only differ in domain, not protocol. SO_REUSEADDR seems to
 * let us get away with this though.
 * In addition, we bind first to the IPv6 wildcard address, which on some
 * systems grabs the IPv4 wildcard address to, thus listening for both, but also
 * blocks attempts to later bind to the IPv6 wildcard address. The order is
 * important, because conversely binding to the IPv4 wildcard doesn't pick up
 * IPv6 connections.
 */
void *indexnodes_listen_main(void *args)
{
    int s4, s6;
    indexnodes_listener_thread_info_t *info = args;


    print_network_interfaces();

    s4 = get_ipv4_socket();
    s6 = get_ipv6_socket();
    listener_thread_event_loop(s4, s6, info->packet_received_cb);


    /* TODO: these almost certainly shouldn't be here */
    if (s4 != -1) close(s4);
    if (s6 != -1) close(s6);

    return NULL;
}

static void print_network_interfaces (void)
{
    struct ifaddrs *ifaddr, *ifa;
    int family, s;


    /* TODO: make this a config option */
    trace_info("Listening on all addresses:\n");

    errno = 0;
    if (getifaddrs(&ifaddr) == -1)
    {
        trace_warn("Cannot get interface addresses: %s\n", strerror(errno));
    }

    for (ifa = ifaddr; ifa; ifa = ifa->ifa_next)
    {
        char host[NI_MAXHOST];

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
}

static int get_ipv6_socket (void)
{
    int s;
    struct sockaddr_in6 sa;
    const int one = 1;
    int bind_rc, rc_ok = 0;


    memset(&sa, 0, sizeof(sa));
    sa.sin6_family = AF_INET6;
    sa.sin6_port   = htons(config_indexnode_advert_port);
    sa.sin6_addr   = in6addr_any;

    errno = 0;
    s = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    if (s != -1)
    {
        errno = 0;
        bind_rc = bind(s, (struct sockaddr *)&sa, sizeof(sa));
        if (!bind_rc)
        {
            rc_ok = 1;
            trace_info("Listening for index node on udp6 port %d...\n", ntohs(sa.sin6_port));
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


    return rc_ok ? s : -1;
}

static int get_ipv4_socket (void)
{
    int s;
    struct sockaddr_in sa;
    const int one = 1;
    int bind_rc, rc_ok = 0;


    memset(&sa, 0, sizeof(sa));
    sa.sin_family      = AF_INET;
    sa.sin_port        = htons(config_indexnode_advert_port);
    sa.sin_addr.s_addr = INADDR_ANY;

    errno = 0;
    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    if (s != -1)
    {
        errno = 0;
        bind_rc = bind(s, (struct sockaddr *)&sa, sizeof(sa));
        if (!bind_rc)
        {
            rc_ok = 1;
            trace_info("Listening for index node on udp4 port %d...\n", ntohs(sa.sin_port));
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


    return rc_ok ? s : -1;
}

static int parse_fs2protocol_version (const char *buf, char **version)
{
    int rc = 1;
    char *v = malloc(strlen(buf) * sizeof(char));

    if (v &&
        sscanf(buf, "fs2protocol-%s", v) == 1)
    {
        *version = v;
        rc = 0;
    }

    return rc;
}

static char *get_next_field (const char **buf)
{
    char *loc, *s = NULL;
    size_t off;

    loc = strchr(*buf, ':');
    if (loc)
    {
        off = loc - *buf;
        s = malloc(off + 1);
        strncpy(s, *buf, off);
        *buf = loc + 1;
    }

    return s;
}

static int parse_advert_packet (const char *buf, char **port, char **version, char **id)
{
    char *version_field, *port_or_auto_field, *weight_field, *id_field;


    version_field = get_next_field(&buf);
    if (!version_field) return 1;
    if (parse_fs2protocol_version(version_field, version)) return 1;

    port_or_auto_field = get_next_field(&buf);
    if (!port_or_auto_field) return 1;
    if (!strcmp(port_or_auto_field, "autoindexnode"))
    {
        /* TODO we don't support autoindex nodes yet. What's their port? */
        return 1;

        weight_field = get_next_field(&buf);
        if (!weight_field) return 1;
        /* *weight = weight_field; */
    }
    else
    {
        *port = port_or_auto_field;
    }

    id_field = get_next_field(&buf);
    if (!id_field) return 1;
    *id = id_field;


    return 0;
}

static void listener_thread_event_loop (int s4, int s6, packet_received_cb_t packet_received_cb)
{
    socklen_t socklen;
    fd_set r_fds;
    int select_rc, recv_rc;


    FD_ZERO(&r_fds);
    if (s4 != -1) FD_SET(s4, &r_fds);
    if (s6 != -1) FD_SET(s6, &r_fds);


    /* TODO: select() needs a timeout (or a pipe to signal exit), otherwise if
     * there are no indexnode packets then this thread will never exit. */
    /* TODO: on shutdown with no indexnodes running, I seem to get stuck in
     * recvfrom() for ip4. No idea how that socket became live if recvfrom()
     * will block. */
    /* FIXME: yeah, shutdown and shit */
    while (1)
    {
        errno = 0;
        char buf[1024], host[NI_MAXHOST];
        string_buffer_t *buffer = string_buffer_new();
        char **port = NULL, **version = NULL, **id = NULL;
        select_rc = select(MAX(s4, s6) + 1, &r_fds, NULL, NULL, NULL);
        struct sockaddr_in  sa4;
        struct sockaddr_in6 sa6;

        switch (select_rc)
        {
            case -1:
                trace_warn("Error waiting for indexnode broadcast: %s\n", strerror(errno));
                break;
            case 0:
                trace_warn("Not found\n");
                break;
            default:
                /* ZOMG: dedupe */
                if (FD_ISSET(s4, &r_fds))
                {
                    socklen = sizeof(sa4);
                    do
                    {
                        recv_rc = recvfrom(s4, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&sa4, &socklen);
                        if (recv_rc > 0)
                        {
                            buf[recv_rc] = '\0';
                            string_buffer_append(buffer, buf);
                        }
                    } while( recv_rc > 0 );

                    if (recv_rc == 0 &&
                        socklen == sizeof(sa4))
                    {
                        if (!parse_advert_packet(string_buffer_peek(buffer), port, version, id) &&
                            inet_ntop(AF_INET, &(sa4.sin_addr), host, NI_MAXHOST))
                        {
                            packet_received_cb(host, *port, *version, *id);
                        }
                    }
                }
                else if (FD_ISSET(s6, &r_fds))
                {
                    socklen = sizeof(sa6);
                    do
                    {
                        recv_rc = recvfrom(s6, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&sa6, &socklen);
                        if (recv_rc > 0)
                        {
                            buf[recv_rc] = '\0';
                            string_buffer_append(buffer, buf);
                        }
                    } while( recv_rc > 0 );

                    if (recv_rc == 0 &&
                        socklen == sizeof(sa6))
                    {
                        if (!parse_advert_packet(string_buffer_peek(buffer), port, version, id) &&
                            inet_ntop(AF_INET6, &(sa6.sin6_addr), host, NI_MAXHOST))
                        {
                            packet_received_cb(host, *port, *version, *id);
                        }
                    }
                }
                else
                {
                    assert(0);
                }

                break;
        }

        string_buffer_delete(buffer);
    }
}
