
#include "common.h"

#include <stdlib.h>

#include "kvp.h"


struct _kvp_t
{
    int key;
    void *value;
};


kvp_t *kvp_new( int key, void *value )
{
    kvp_t *kvp = malloc( sizeof(*kvp) );

    kvp->key = key;
    kvp->value = value;

    return kvp;
}

void kvp_delete( kvp_t *kvp )
{
    free( kvp );
}


int kvp_key( kvp_t *kvp )
{
    return kvp->key;
}

void *kvp_value( kvp_t *kvp )
{
    return kvp->value;
}
