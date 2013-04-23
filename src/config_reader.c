
#include "common.h"

#include <stdlib.h>

#include "config_reader.h"
#include "config_internal.h"

/* TODO:
 * as many readers as you like. they don't hold state and don't change anything
 * one "data store" stack which
 * - can have data values "added" - stack up immutible version (e.g. defaults,
 *   from /etc, from ~, cmdline) which are scanned in order
 * - entries in data_t need to know if they're "present" or not - that config
 *   file or the cmd line may not have specified them, in which case move onto
 *   the next level
 *   - -1 arg to _from_cmdline means "not set?"
 *   - Think about char ** (have a del() fn, just add a copy() one?)
 * - think about the freeing of all of this stuff!
 * - this would allow the changable ones (like those read from disk) to be
 *   replaced in repsonse to SIGHUP but leave higher over-rides (like cmdline)
 * - test this
 *   - don't add anything, check defaults
 *   - add manually, check over-rides
 *   - add a file, check parsing
 *   - add several files and overrides, check priority
 * - remove config-config.xml and instead have defaults.conf which is walked
 *   (must be a fn to get the xpath for the current node). Annotoate the nodes
 *   with anything else needed to build the code (just ctype?)
 * FIXME: reader functions just returning defulats from the bottom of the stack
 * for now - need the missing flag
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
