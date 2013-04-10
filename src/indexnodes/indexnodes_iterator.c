/*
 * Copyright (C) 2008-2013 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Indexnodes list iterator class.
 */

#include "common.h"

#include <stdlib.h>
#include <string.h>

#include "indexnodes_iterator.h"

#include "indexnodes_list_internal.h"


struct _indexnodes_iterator_t
{
    item_t *current;
};


indexnodes_iterator_t *indexnodes_iterator_begin( indexnodes_list_t *ins )
{
    indexnodes_iterator_t *iter = calloc( sizeof(struct _indexnodes_iterator_t), 1 );


    iter->current = TAILQ_FIRST(&ins->list);


    return iter;
}

indexnode_t *indexnodes_iterator_current( indexnodes_iterator_t *iter )
{
    return indexnode_post( CALLER_INFO iter->current->in );
}

indexnodes_iterator_t *indexnodes_iterator_next( indexnodes_iterator_t *iter_old )
{
    indexnodes_iterator_t *iter_new = calloc( sizeof(struct _indexnodes_iterator_t), 1 );


    iter_new->current = TAILQ_NEXT(iter_old->current, next);
    free( iter_old );


    return iter_new;
}

int indexnodes_iterator_end( indexnodes_iterator_t *iter )
{
    return iter->current == NULL;
}

void indexnodes_iterator_delete( indexnodes_iterator_t *iter )
{
    free( iter );
}
