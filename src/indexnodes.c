/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * The collection of known indexnodes.
 * This module spanws a thread that listens for indexnode broadcats and
 * maintains a list.
 */

/* TODO:
 * this assumes indexnodes live forever. Need to have the thread try to match
 * broadcasts against existing indexnodes. Unknown indexnodes are new. Known
 * indexnodes have their timeout put back to 0. Also needs to be a thread on a
 * timer that goes over all the indexnodes. If the timout has exceeded some
 * constant, change its state and remove it from the list, and dec ref count.
 * Anything that's still using it (they're ref-counted) sees it's in state dead,
 * and should error and return asap, deleting the indexnode as they do so.
 * For refcount to hit 0, the indexnode must not be owned by the list (assert
 * this) and so it can be free()d
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ifaddrs.h>

#include "queue.h"

#include "common.h"
#include "config.h"
#include "indexnodes.h"
#include "fetcher.h"
#include "string_buffer.h"
#include "utils.h"


typedef struct _listener_thread_info_t
{
    pthread_t pthread_id;
} listener_thread_info_t;


static int s_exiting = 0; /* TODO: enum state machine */
static listener_thread_info_t *s_listener_thread_info;
static indexnodes_list_t *s_indexnodes;
static pthread_mutex_t s_indexnodes_lock; /* TODO: rwlock */

static void *indexnodes_listen_main(void *args);

static char *version_cb (const proto_indexnode_t *in, char *buf);


int indexnodes_init (void)
{
    s_indexnodes = indexnodes_list_new();


    pthread_mutex_init(&s_indexnodes_lock, NULL);

    /* Instantiate statically configured indexnodes */
    int i = 0;
    const char *host, *port, *version;
    while ((host = config_indexnode_hosts[i]) &&
           (port = config_indexnode_ports[i]))
    {
        const proto_indexnode_t *pin = proto_indexnode_new(host, port);

        /* TODO: should do this async, in parallel, so that it happens as fast
         * as possible and that we can get on with other stuff straight away.
         * The following logic would have to move to a callback. */
        version = fetcher_get_indexnode_version(pin, &version_cb); /* blocks */

        indexnode_t *in = indexnode_from_proto(CALLER_INFO pin, version);
        if (in)
        {
            /* Currently no way to get the id of a static indexnode (if you
             * can't see its broadcasts, so we just add them and assume there
             * are no duplicates */
            indexnodes_list_add(s_indexnodes, in);

            trace_info(
                "Static index node configured at %s:%s - version %s\n",
                indexnode_host(in),
                indexnode_port(in),
                indexnode_version(in));
        }

        i++;
    }

    return 0;
}

void indexnodes_finalise (void)
{
    /* TODO: assert the thread state == stopped */
    /* TODO: should we do this, or should the thread bump their statemachine to
     * dead, let them die, then just assert they're dead here? */
    indexnodes_list_delete(s_indexnodes);

    pthread_mutex_destroy(&s_indexnodes_lock);

    free(s_listener_thread_info);
}

void indexnodes_start_listening (void)
{
    s_listener_thread_info = (listener_thread_info_t *)malloc(sizeof(listener_thread_info_t));

    /* Start listening for broadcasting indexnodes */
    assert(!pthread_create(&(s_listener_thread_info->pthread_id), NULL, &indexnodes_listen_main, NULL));
}

void indexnodes_stop_listening (void)
{
    /* Signal and wait for thread */
    s_exiting = 1;
    pthread_join(s_listener_thread_info->pthread_id, NULL);
}

 /* TODO: If de's hold indexnodes then they either need to live for ever in state DEAD,
 * or be ref-counted and die. I suggest they're refcounted, and don't work when
 * in state dead. If ref-count hits 0 AND they're dead then they can die. de's
 * with DEAD indexnodes are dead themselves - when they discover that they
 * should act like they did a fetch and got a 404
 *
 * de's should make their own URIs and have their own fetch and list functions,
 * deferring to the right place. The ops should really only interact with them.
 * For now, still get the list of indexnodes for stat() at least.
 */
indexnodes_list_t *indexnodes_get (void)
{
    indexnodes_list_t *ins;


    pthread_mutex_lock(&s_indexnodes_lock);
    /* currently only happens lazily, here. Might be worth putting on a bg
     * thread, if only for debugging */
    s_indexnodes = indexnodes_list_remove_expired(s_indexnodes);
    ins = indexnodes_list_copy(s_indexnodes);
    pthread_mutex_unlock(&s_indexnodes_lock);


    return ins;
}


static int indexnode_parse_version (const char *buf, char **version)
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
    char *tmp;


    tmp = get_next_field(&buf);
    if (!tmp) return 1;
    if (indexnode_parse_version(tmp, version)) return 1;

    tmp = get_next_field(&buf);
    if (!tmp) return 1;
    if (!strcmp(tmp, "autoindexnode"))
    {
        /* FIXME we don't support autoindex nodes yet. What's their port? */
        return 1;

        tmp = get_next_field(&buf);
        if (!tmp) return 1;
        /* *weight = tmp; */
    }
    else
    {
        *port = tmp;
    }

    tmp = get_next_field(&buf);
    if (!tmp) return 1;
    *id = tmp;


    return 0;
}

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
static void *indexnodes_listen_main(void *args)
{
    indexnode_t *in;
    int s4 = -1, s6 = -1;
    int s4_ok = 0, s6_ok = 0;
    int their_rc;
    socklen_t socklen;
    struct sockaddr_in  sa4;
    struct sockaddr_in6 sa6;
    fd_set r_fds;
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    const int one = 1;


    NOT_USED(args);

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


    /* ==== Set up listening sockets ==== */

    /* IPv6 */
    memset(&sa6, 0, sizeof(sa6));
    sa6.sin6_family = AF_INET6;
    sa6.sin6_port   = htons(42444); /* TODO: config option */
    sa6.sin6_addr   = in6addr_any;

    errno = 0;
    s6 = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    /* See function comment */
    setsockopt(s6, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
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

    /* IPv4 */
    memset(&sa4, 0, sizeof(sa4));
    sa4.sin_family      = AF_INET;
    sa4.sin_port        = htons(42444); /* TODO: config option */
    sa4.sin_addr.s_addr = INADDR_ANY;

    errno = 0;
    s4 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    /* See function comment */
    setsockopt(s4, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
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


    /* ==== Wait for broadcast packets ==== */

    FD_ZERO(&r_fds);
    if (s4_ok) FD_SET(s4, &r_fds);
    if (s6_ok) FD_SET(s6, &r_fds);

    /* TODO: select() needs a timeout (or a pipe to signal exit), otherwise if
     * there are no indexnode packets then this thread will never exit. */
    /* TODO: on shutdown with no indexnodes running, I seem to get stuck in
     * recvfrom() for ip4. No idea how that socket became live if recvfrom()
     * will block. */
    while (!s_exiting)
    {
        int found = 0;
        errno = 0;
        char buf[1024], host[NI_MAXHOST];
        string_buffer_t *buffer = string_buffer_new();
        char **port = NULL, **version = NULL, **id = NULL;
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
                    do
                    {
                        their_rc = recvfrom(s4, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&sa4, &socklen);
                        if (their_rc > 0)
                        {
                            buf[their_rc] = '\0';
                            string_buffer_append(buffer, buf);
                        }
                    } while( their_rc > 0 );

                    if (their_rc == 0 &&
                        socklen == sizeof(sa4))
                    {
                        if (!parse_advert_packet(string_buffer_peek(buffer), port, version, id) &&
                            inet_ntop(AF_INET, &(sa4.sin_addr), host, NI_MAXHOST))
                        {
                            found = 1;
                        }
                    }
                }
                else if (FD_ISSET(s6, &r_fds))
                {
                    socklen = sizeof(sa6);
                    do
                    {
                        their_rc = recvfrom(s6, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&sa6, &socklen);
                        if (their_rc > 0)
                        {
                            buf[their_rc] = '\0';
                            string_buffer_append(buffer, buf);
                        }
                    } while( their_rc > 0 );

                    if (their_rc == 0 &&
                        socklen == sizeof(sa6))
                    {
                        if (!parse_advert_packet(string_buffer_peek(buffer), port, version, id) &&
                            inet_ntop(AF_INET6, &(sa6.sin6_addr), host, NI_MAXHOST))
                        {
                            found = 1;
                        }
                    }
                }
                else
                {
                    assert(0);
                }

                break;
        }

        if (found)
        {
            in = indexnode_new(CALLER_INFO host, *port, *id, *version);

            if (in)
            {
                pthread_mutex_lock(&s_indexnodes_lock);
                if (!indexnodes_list_find(s_indexnodes, indexnode_id(in)))
                {
                    /* If it's not been seen before, add it */
                    indexnodes_list_add(s_indexnodes, in);
                }
                else
                {
                    /* TODO: reset timeout */
                }
                pthread_mutex_unlock(&s_indexnodes_lock);

                trace_info("Found index node, version %s, at %s:%s (id: %s)\n", *version, host, *port, *id);
            }
        }

        string_buffer_delete(buffer);
    }


    if (s4_ok) close(s4);
    if (s6_ok) close(s6);

    return NULL;
}

static char *version_cb (const proto_indexnode_t *in, char *buf)
{
    char *version = NULL;

    assert(in);
    assert(buf); assert(*buf);

    indexnode_parse_version(buf, &version);

    return version;
}
