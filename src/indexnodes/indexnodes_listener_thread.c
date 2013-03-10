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
static void listener_thread_event_loop (int s4, int s6, int control_fd, packet_received_cb_t packet_received_cb, void *packet_received_ctxt);


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
    listener_thread_args_t *info = args;


    print_network_interfaces();

    s4 = get_ipv4_socket();
    s6 = get_ipv6_socket();

    listener_thread_event_loop(s4, s6, info->control_fd, info->packet_received_cb, info->packet_received_ctxt);


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

static int get_socket (struct sockaddr *sa, int domain, socklen_t socklen_in)
{
    int s;
    const int one = 1;
    int bind_rc, rc_ok = 0;


    errno = 0;
    s = socket(domain, SOCK_DGRAM, IPPROTO_UDP);
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    if (s != -1)
    {
        errno = 0;
        bind_rc = bind(s, sa, socklen_in);
        if (!bind_rc)
        {
            rc_ok = 1;
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

static int get_ipv4_socket (void)
{
    int s;
    struct sockaddr_in sa;
    const socklen_t socklen = sizeof(sa);
    const int domain = AF_INET;


    memset(&sa, 0, socklen);

    sa.sin_family      = domain;
    sa.sin_port        = htons(config_indexnode_advert_port);
    sa.sin_addr.s_addr = INADDR_ANY;

    s = get_socket((struct sockaddr *)&sa, socklen, domain);
    trace_info("Listening for index node on udp4 port %d...\n", ntohs(sa.sin_port));


    return s;
}

static int get_ipv6_socket (void)
{
    int s;
    struct sockaddr_in6 sa;
    const socklen_t socklen = sizeof(sa);
    const int domain = AF_INET6;


    memset(&sa, 0, socklen);

    sa.sin6_family      = domain;
    sa.sin6_port        = htons(config_indexnode_advert_port);
    sa.sin6_addr        = in6addr_any;

    s = get_socket((struct sockaddr *)&sa, socklen, domain);
    trace_info("Listening for index node on udp6 port %d...\n", ntohs(sa.sin6_port));


    return s;
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

static int parse_advert_packet (const char *buf, char **port, char **fs2protocol, char **id)
{
    char *version_field, *port_or_auto_field, *weight_field, *id_field;


    version_field = get_next_field(&buf);
    if (!version_field) return 1;
    *fs2protocol = version_field;

    port_or_auto_field = get_next_field(&buf);
    if (!port_or_auto_field) return 1;
    if (!strcmp(port_or_auto_field, "autoindexnode"))
    {
        free(port_or_auto_field);

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

static void receive_advert(
    const int socket,
    struct sockaddr *sa,
    const socklen_t socklen_in,
    void * const addr_src,
    const size_t host_len,
    packet_received_cb_t packet_received_cb,
    void *packet_received_ctxt
)
{
    char host[host_len];
    char buf[1024];
    char **port = NULL, **fs2protocol = NULL, **id = NULL;
    string_buffer_t *buffer = string_buffer_new();
    int recv_rc;
    socklen_t socklen = socklen_in;


    do
    {
        recv_rc = recvfrom(socket, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&sa, &socklen);
        if (recv_rc > 0)
        {
            buf[recv_rc] = '\0';
            string_buffer_append(buffer, buf);
        }
    } while( recv_rc > 0 );

    if (recv_rc == 0 &&
        socklen == socklen_in)
    {
        if (!parse_advert_packet(string_buffer_peek(buffer), port, fs2protocol, id) &&
            inet_ntop(AF_INET, addr_src, host, sizeof(host) - 1))
        {
            packet_received_cb(packet_received_ctxt, host, *port, *fs2protocol, *id);
        }
    }

    string_buffer_delete(buffer);
}

static void listener_thread_event_loop (int s4, int s6, int control_fd, packet_received_cb_t packet_received_cb, void *packet_received_ctxt)
{
    fd_set r_fds;
    int select_rc;
    int exiting = 0;


    FD_ZERO(&r_fds);
    if (s4 != -1) FD_SET(s4, &r_fds);
    if (s6 != -1) FD_SET(s6, &r_fds);
    assert(control_fd != -1); FD_SET(control_fd, &r_fds);


    while (!exiting)
    {
        errno = 0;
        select_rc = select(MAX(s4, MAX(s6, control_fd)) + 1, &r_fds, NULL, NULL, NULL);

        switch (select_rc)
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
                    struct sockaddr_in sa;
                    receive_advert(s4, (struct sockaddr *)&sa, sizeof(sa), &(sa.sin_addr), INET_ADDRSTRLEN, packet_received_cb, packet_received_ctxt);
                }
                else if (FD_ISSET(s6, &r_fds))
                {
                    struct sockaddr_in6 sa;
                    receive_advert(s6, (struct sockaddr *)&sa, sizeof(sa), &(sa.sin6_addr), INET6_ADDRSTRLEN, packet_received_cb, packet_received_ctxt);
                }
                else if (FD_ISSET(control_fd, &r_fds))
                {
                    /* For now, only one command, STOP_LISTENING, represented by
                     * the unit-typed message */
                    exiting = 1;
                }
                else
                {
                    assert(0);
                }

                break;
        }
    }
}
