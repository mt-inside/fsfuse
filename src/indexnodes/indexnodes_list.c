/*
 * Copyright (C) 2008-2013 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TODO: split this into indexnodes_set which is the google structure and
 * internal to the indexnodes module. AND indexnodes_list which is what's
 * produced and returned to consumers. This really can be a generic_list<> (as
 * it will have no methods other than those to support enumeration), or a
 * typedef of a LINKED_LIST (no need for TAILQ)
 * TODO: internally this needs to be a google-interview data structure :)
 * - Array with hash over it (maybe pull some logic out of hash like a hasher
 *   class)
 * - O(1) add, delete, find-by-uid
 * No. no need to delete or get-random, so can just be a hash table
 */

#include "common.h"

#include <stdlib.h>
#include <string.h>

#include "indexnodes_list_internal.h"

#include "indexnode.h"
#include "indexnode_internal.h"


indexnodes_list_t *indexnodes_list_new (void)
{
    indexnodes_list_t *ins = (indexnodes_list_t *)calloc(1, sizeof(indexnodes_list_t));


    TAILQ_INIT(&ins->list);


    return ins;
}

void indexnodes_list_add (indexnodes_list_t *ins,
                          indexnode_t *in)
{
    item_t *item = NULL;


    item = (item_t *)malloc(sizeof(item_t));
    item->in = in;

    TAILQ_INSERT_HEAD(&ins->list, item, next);
}

indexnode_t *indexnodes_list_find (CALLER_DECL indexnodes_list_t *ins, const char * const id)
{
    item_t *item;
    indexnode_t *in = NULL;
    int found = 0;


    TAILQ_FOREACH(item, &ins->list, next)
    {
        if (indexnode_equals(item->in, id))
        {
            in = indexnode_post( CALLER_PASS item->in );
            found = 1;
        }
        if (found) break;
    }


    return in;
}

indexnodes_list_t *indexnodes_list_copy (CALLER_DECL indexnodes_list_t *orig)
{
    indexnodes_list_t *ret = indexnodes_list_new();
    item_t *item;


    TAILQ_FOREACH(item, &orig->list, next)
    {
        indexnodes_list_add(ret, indexnode_post(CALLER_PASS item->in));
    }


    return ret;
}

indexnodes_list_t *indexnodes_list_remove_expired (CALLER_DECL indexnodes_list_t *orig)
{
    indexnodes_list_t *ret = indexnodes_list_new();
    item_t *item;


    TAILQ_FOREACH(item, &orig->list, next)
    {
        if (indexnode_still_valid(item->in))
        {
            indexnodes_list_add(ret, indexnode_post(CALLER_PASS item->in));
        }
        else
        {
            /* simply don't post here, and they will be deleted when the old
             * list is deleted */
        }
    }

    indexnodes_list_delete(orig);


    return ret;
}

void indexnodes_list_delete (indexnodes_list_t *ins)
{
    item_t *item, *tmp_item;


    TAILQ_FOREACH_SAFE(item, &ins->list, next, tmp_item)
    {
        indexnode_delete(CALLER_INFO item->in);
        free(item);
    }

    free(ins);
}
