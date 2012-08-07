/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Indexnode class.
 */

 /* NB: this file relies on _POSIX_SOURCE to get NI_MAX[HOST|etc], even though
  * the man page and netdb.h seem to say it really should be _BSD_SOURCE */

#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "config.h"
#include "indexnode.h"
#include "fetcher.h"


struct _indexnode_t
{
    char *host;
    char *port;
    char *version;
    char *id;
};


/* TODO: this shouldn't be mutable, data to be passed to ctor */
/* TODO: state machine */
indexnode_t *indexnode_new (void)
{
    indexnode_t *in;


    in = calloc(sizeof(indexnode_t), 1);


    return in;
}

void indexnode_delete (indexnode_t *in)
{
    if (in->host)    free(in->host);
    if (in->port)    free(in->port);
    if (in->version) free(in->version);
    if (in->id)      free(in->id);

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

void indexnode_set_id (indexnode_t *in, char *id)
{
    in->id = strdup(id);
}
char *indexnode_get_id (indexnode_t *in)
{
    return in->id;
}
