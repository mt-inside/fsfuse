/*
 * Indexnode class.
 *
 * Copyright (C) Matthew Turner 2008-2011. All rights reserved.
 *
 * $Id$
 */

 /* NB: this file relies on _POSIX_SOURCE to get NI_MAX[HOST|etc], even though
  * the man page and netdb.h seem to say it really should be _BSD_SOURCE */

#include <stdlib.h>
#include <string.h>
#include <netdb.h>

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


indexnode_t *indexnode_new (void)
{
    indexnode_t *in;


    in = malloc(sizeof(indexnode_t));
    in->host    = (char *)malloc(NI_MAXHOST * sizeof(char));
    in->port    = (char *)malloc(NI_MAXSERV * sizeof(char));
    in->version = (char *)malloc(1024       * sizeof(char));


    return in;
}

void indexnode_delete (indexnode_t *in)
{
    free(in->host);
    free(in->port);
    free(in->version);

    free(in);
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
