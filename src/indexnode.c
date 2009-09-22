/*
 * Code for interacting with indexnodes.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "config.h"
#include "indexnode.h"
#include "fetcher.h"


#define VERSION_LEN 1024
#define HOST_LEN 1024
#define PORT_LEN 1024


/* these are global, but they are written before any threads are spawned, and
 * thereafter only read */
/* statically allocated because libcurl simply copies the pointers, so these
 * must stay in scope. */
static char *host, *port;
static double version = 0.0;


int indexnode_find (void)
{
    /* TODO: security */
    int s = -1;
    int their_rc, rc = 1;
    socklen_t socklen;
    char buf[1024], version_str[1024];
    struct sockaddr_in sa;


    if (!config_indexnode_autodetect)
    {
        host = config_indexnode_host;
        port = config_indexnode_port;
        version = fetcher_get_indexnode_version();

        printf("Static index node configured at %s:%s - version %f\n", host, port, version);

        return 0;
    }


    sa.sin_family      = AF_INET;
    sa.sin_port        = htons(42444);
    sa.sin_addr.s_addr = INADDR_ANY;

    printf("Listening for index node on port %d...\n", ntohs(sa.sin_port));

    host    = (char *)malloc(HOST_LEN    * sizeof(char));
    port    = (char *)malloc(PORT_LEN    * sizeof(char));

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s != -1)
    {
        their_rc = bind(s, (struct sockaddr *)&sa, sizeof(sa));
        if (!their_rc)
        {
            socklen = sizeof(sa);
            their_rc = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&sa, &socklen);
            if (their_rc > 0 &&
                socklen == sizeof(sa))
            {
                buf[their_rc] = '\0';

                sscanf(buf, "%[^:]:%s", version_str, port);
                version = indexnode_parse_version(version_str);

                if (inet_ntop(AF_INET, &(sa.sin_addr), host, HOST_LEN))
                {
                    rc = 0;
                    printf("Found index node, version %f, at %s:%s\n", version, host, port);
                }
            }
        }
        else
        {
            printf("Cannot bind to indexnode listener socket\n");
        }

        close(s);
    }


    return rc;
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
