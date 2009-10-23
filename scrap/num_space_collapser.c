#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "num_space_collapser.h"
#include "hash.h"


struct _num_space_collapser_t
{
    hash_table_t *hash_table;
    pthread_mutex_t mutex;
};


static unsigned next_value = 0;


num_space_collapser_t *num_space_collapser_new (void)
{
    num_space_collapser_t *nsc = (num_space_collapser_t *)calloc(sizeof(num_space_collapser_t), 1);


    pthread_mutex_init(&nsc->mutex, NULL);
    nsc->hash_table = hash_table_new(32);


    return nsc;
}

void num_space_collapser_delete (num_space_collapser_t *nsc)
{
    hash_table_delete(nsc->hash_table);
    pthread_mutex_destroy(&nsc->mutex);

    free(nsc);
}

unsigned num_space_collapser_get (num_space_collapser_t *nsc, unsigned key)
{
    unsigned *val;
    char key_str[1024];


    pthread_mutex_lock(&nsc->mutex);

    sprintf(key_str, "%u", key);

    if ((val = (unsigned *)hash_table_find(nsc->hash_table, key_str)))
    {
        printf("returning mapping %s -> %u\n", key_str, *val);
    }
    else
    {
        val = (unsigned *)malloc(sizeof(unsigned));
        *val = next_value++;
        printf("adding mapping %s -> %u\n", key_str, *val);
        hash_table_add(nsc->hash_table, strdup(key_str), (void *)val);
    }

    pthread_mutex_unlock(&nsc->mutex);

    return *val;
}
