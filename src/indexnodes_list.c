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

#include "common.h"

#include <stdlib.h>

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

/* TODO: don't return lists directly, have an itterator class.
 * When that's done, copy() can be internal, I think (or the iterator just ref
 * counts the list).
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
