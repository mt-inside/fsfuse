
#include "common.h"

#include <stdlib.h>

#include "config_reader.h"
#include "config_internal.h"

/* TODO:
 * as many readers as you like. they don't hold state and don't change anything
 * one "data store" stack which
 * - add datas from multiple files, e.g. /etc, ~
 * - think about the freeing of all of this stuff!
 *   - Think about char ** (have a del() fn, just add a copy() one?)
 * - respond to SIGHUP - re-load the mutable ones (they need marking some how,
 *   and to remember their origin file)
 * - test this
 *   - don't add anything, check defaults
 *   - add manually, check over-rides
 *   - add a file, check parsing
 *   - add several files and overrides, check priority
 * - remove config-config.xml and instead have defaults.conf which is walked
 *   (must be a fn to get the xpath for the current node). Annotoate the nodes
 *   with anything else needed to build the code (just ctype?)
 */

config_reader_t *config_reader_new( config_data_t **datas, size_t datas_len )
{
    config_reader_t *config = malloc( sizeof(*config) );


    config->datas = datas;
    config->datas_len = datas_len;


    return config;
}

void config_reader_delete( config_reader_t *config )
{
    free( config );
}
