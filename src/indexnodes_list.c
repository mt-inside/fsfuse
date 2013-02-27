/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * A list type for a list of indexnodes.
 * TODO: now this has been factored out, is is so simple it can be merged back
 * into indexnodes.c? Especially if indexnodes have an intrinsic list (that's
 * kept private to in/ins via an indexnode_internal.h
 */

#include "common.h"

#include <stdlib.h>
#include <string.h>

#include "indexnodes_list.h"

#include "indexnode.h"


indexnodes_list_t *indexnodes_list_new (void)
{
    indexnodes_list_t *ins = (indexnodes_list_t *)calloc(1, sizeof(indexnodes_list_t));


    TAILQ_INIT(&ins->list);


    return ins;
}

void indexnodes_list_add (indexnodes_list_t *ins,
                          indexnode_t *in)
{
    indexnodes_list_item_t *item = NULL;


    item = (indexnodes_list_item_t *)malloc(sizeof(indexnodes_list_item_t));

    item->in = indexnode_post(in);
    TAILQ_INSERT_HEAD(&ins->list, item, next);
}

/* TODO: take a bool (*indexnode_matcher)(in) instead of this id */
indexnode_t *indexnodes_list_find (indexnodes_list_t *ins, const char * const id)
{
    indexnodes_list_item_t *item;
    indexnode_t *in = NULL;
    const char *item_id;
    int found = 0;


    TAILQ_FOREACH(item, &ins->list, next)
    {
        item_id = indexnode_id(item->in);
        if (item_id && !strcmp(item_id, id))
        {
            in = item->in;
            found = 1;
        }
        free_const(item_id);
        if (found) break;
    }


    return in;
}


/* TODO: don't return lists directly, have an itterator class.
 * When that's done, copy() can be internal, I think (or the iterator just ref
 * counts the list).
 * It's really nasty that fetcher, etc see the internals of
 * this
 */
indexnodes_list_t *indexnodes_list_copy (indexnodes_list_t *orig)
{
    indexnodes_list_t *ret = indexnodes_list_new();
    indexnodes_list_item_t *item;


    TAILQ_FOREACH(item, &orig->list, next)
    {
        indexnodes_list_add(ret, indexnode_post(item->in));
    }


    return ret;
}

/* TODO: make remove_where(Predicate<indexnode>) */
indexnodes_list_t *indexnodes_list_remove_expired (indexnodes_list_t *orig)
{
    indexnodes_list_t *ret = indexnodes_list_new();
    indexnodes_list_item_t *item;


    TAILQ_FOREACH(item, &orig->list, next)
    {
        if (indexnode_still_valid(item->in))
        {
            indexnodes_list_add(ret, indexnode_post(item->in));
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
    indexnodes_list_item_t *item;


    TAILQ_FOREACH(item, &ins->list, next)
    {
        free(item);
        indexnode_delete(item->in);
    }

    free(ins);
}