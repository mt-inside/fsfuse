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
 * When a copy of the list is given out it only contains live indexnodes.
 * At the same time, this class gives up its copy of any dead indexnodes so that
 * they will eventually be free()d.
 * This is done lazily, rather than pro-actively. Yes, this is a slight memory
 * leak if indexnodes die and no-one fetches the list for ages, but it saves a
 * load of complexity.
 */

#include "common.h"

#include <stdlib.h>
#include <string.h>

#include "indexnodes.h"
#include "indexnodes_internal.h"

#include "indexnode_internal.h"
#include "indexnodes_list_internal.h"
#include "indexnodes_listener.h"
#include "indexnodes_statics_manager.h"

#include "fetcher.h"
#include "locks.h"
#include "string_buffer.h"
#include "utils.h"


struct _indexnodes_t
{
    indexnodes_list_t *list;
    indexnodes_statics_manager_t *statics;
    indexnodes_listener_t *listener;
    rw_lock_t *lock;
};


static void new_indexnode_event (
    const void *ctxt,
    const char *host,
    const char *port,
    const char *version,
    const char *id
);


indexnodes_t *indexnodes_new (void)
{
    indexnodes_t *ins = malloc(sizeof(indexnodes_t));


    ins->lock = rw_lock_new();
    ins->list = indexnodes_list_new();

    ins->statics = indexnodes_statics_manager_new(&new_indexnode_event, ins);
    ins->listener = indexnodes_listener_new(&new_indexnode_event, ins);


    return ins;
}

void indexnodes_delete (indexnodes_t *ins)
{
    indexnodes_listener_delete(ins->listener);
    indexnodes_statics_manager_delete(ins->statics);

    indexnodes_list_delete(ins->list);
    rw_lock_delete(ins->lock);

    free(ins);
}

/* de's should make their own URIs and have their own fetch and list functions,
 * deferring to the right place. The ops should really only interact with them.
 * For now, still get the list of indexnodes for stat() at least.
 */
indexnodes_list_t *indexnodes_get (CALLER_DECL indexnodes_t *ins)
{
    indexnodes_list_t *list;


    rw_lock_rlock(ins->lock);
    ins->list = indexnodes_list_remove_expired(CALLER_PASS ins->list);
    list = indexnodes_list_copy(CALLER_PASS ins->list);
    rw_lock_runlock(ins->lock);


    return list;
}


static int parse_fs2protocol (const char *fs2protocol, const char **version)
{
    int rc = 1;
    const char *prefix = "fs2protocol-";
    const size_t prefix_len = strlen(prefix);

    if (!strncmp(fs2protocol, prefix, prefix_len))
    {
        char *v = malloc((strlen(fs2protocol) - prefix_len + 1) * sizeof(char));

        assert(sscanf(fs2protocol, "fs2protocol-%s", v) == 1);

        *version = v;
        rc = 0;
    }

    free_const(fs2protocol);

    return rc;
}

static void new_indexnode_event (
    const void *ctxt,
    const char *host,
    const char *port,
    const char *fs2protocol,
    const char *id
)
{
    indexnodes_t *ins = (indexnodes_t *)ctxt;
    indexnode_t *found_in, *new_in = NULL;
    const char *version;


    if (!parse_fs2protocol(fs2protocol, &version))
    {
        trace_info("Seen advert for indexnode %s at %s:%s (version %s)\n", id, host, port, version);

        if ((found_in = indexnodes_list_find(CALLER_INFO ins->list, id)))
        {
            indexnode_seen(found_in);

            indexnode_delete(CALLER_INFO found_in);
        }
        else
        {
            new_in = indexnode_new(CALLER_INFO host, port, version, id);
            if (new_in)
            {
                rw_lock_wlock(ins->lock);

                /* If it's not been seen before, add it */
                indexnodes_list_add(ins->list, new_in);

                rw_lock_wunlock(ins->lock);
            }
        }
    }

    if (!new_in)
    {
        free_const(host); free_const(port); free_const(version); free_const(id);
    }
}
