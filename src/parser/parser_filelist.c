
#include "common.h"

#include <stdlib.h>
#include <string.h>

#include "parser_filelist.h"

#include "fs2_constants.h"
#include "nativefs.h"
#include "parser_xml.h"


typedef enum
{
    state_WAITING_FOR_DIV_FILELIST,
    state_WAITING_FOR_A,
    state_CONSUMING_A
} state_t;


struct _parser_filelist_t
{
    nativefs_entry_found_cb_t cb;
    void *cb_ctxt;
    parser_xml_t *xml;
    state_t state;

    const char *name, *hash, *type, *href, *client;
    off_t size; unsigned long link_count;
};


void filelist_xml_cb( void *ctxt, parser_xml_event_t event, char *text, char *id );


parser_filelist_t *parser_filelist_new( nativefs_entry_found_cb_t cb, void *ctxt )
{
    parser_filelist_t *parser = calloc( 1, sizeof(*parser) );


    parser->cb = cb;
    parser->cb_ctxt = ctxt;
    parser->xml = parser_xml_new( &filelist_xml_cb, parser );


    return parser;
}

void parser_filelist_delete( parser_filelist_t *parser )
{
    parser_xml_delete( parser->xml );
    free( parser );
}

int parser_filelist_consume( parser_filelist_t *parser, void *data, size_t len )
{
    return parser_xml_consume( parser->xml, data, len );
}

void filelist_xml_cb( void *ctxt, parser_xml_event_t event, char *name, char *value ) //TODO: should be static, fix tests
{
    parser_filelist_t *parser = (parser_filelist_t *)ctxt;


    switch( parser->state )
    {
        case state_WAITING_FOR_DIV_FILELIST:
            if( event == parser_xml_event_TAG_START &&
                !strcmp( name, "div" ) &&
                value != NULL &&
                !strcmp( value, fs2_filelist_node_id ) )
            {
                parser->state = state_WAITING_FOR_A;
            }
            break;
        case state_WAITING_FOR_A:
            if( event == parser_xml_event_TAG_START &&
                !strcmp( name, "a" ) )
            {
                parser->state = state_CONSUMING_A;
            }
            break;
        case state_CONSUMING_A:
            if( event == parser_xml_event_ATTRIBUTE )
            {
                if( !strcmp( name, fs2_name_attribute_key ) )
                {
                    parser->name = strdup( value );
                }
                else if( !strcmp( name, fs2_hash_attribute_key ) )
                {
                    parser->hash = strdup( value );
                }
                else if( !strcmp( name, fs2_type_attribute_key ) )
                {
                    /* TODO: y u no parse to enum here? */
                    parser->type = strdup( value );
                }
                else if( !strcmp( name, fs2_size_attribute_key ) )
                {
                    parser->size = atoll( value );
                }
                else if( !strcmp( name, fs2_linkcount_attribute_key) ||
                         !strcmp( name, fs2_alternativescount_attribute_key ) )
                {
                    parser->link_count = atol( value );
                }
                else if( !strcmp( name, fs2_href_attribute_key ) )
                {
                    parser->href = strdup( value );
                }
                else if( !strcmp( name, fs2_clientalias_attribute_key ) )
                {
                    parser->client = strdup( value );
                }
                else if( !strcmp( name, fs2_path_attribute_key ) )
                {
                    /* ignore what the indexnode says the path is for now */
                }
            }
            if( event == parser_xml_event_TAG_END &&
                !strcmp( name, "a" ) )
            {
                parser->cb(
                    parser->cb_ctxt,
                    parser->hash,
                    parser->name,
                    parser->type,
                    parser->size,
                    parser->link_count,
                    parser->href,
                    parser->client
                );

                parser->state = state_WAITING_FOR_A;
            }
            break;
    }

    free(name);
    free(value);
}
