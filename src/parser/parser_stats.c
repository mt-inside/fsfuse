
#include "common.h"

#include <stdlib.h>
#include <string.h>

#include "parser_stats.h"

#include "parser_xml.h"


typedef enum
{
    state_WAITING_FOR_DIV_GENERAL,
    state_WAITING_FOR_SPAN_WITH_ID,
    state_CONSUMING_FILE_COUNT,
    state_CONSUMING_TOTAL_SIZE
} state_t;


struct _parser_stats_t
{
    parser_stats_cb_t cb;
    void *cb_ctxt;
    parser_xml_t *xml;
    state_t state;
    unsigned long files;
    unsigned long bytes;
};


void stats_xml_cb( void *ctxt, parser_xml_event_t event, char *text, char *id );


parser_stats_t *parser_stats_new( parser_stats_cb_t cb, void *ctxt )
{
    parser_stats_t *parser = calloc( 1, sizeof(*parser) );


    parser->cb = cb;
    parser->cb_ctxt = ctxt;
    parser->xml = parser_xml_new( &stats_xml_cb, parser );


    return parser;
}

void parser_stats_delete( parser_stats_t *parser )
{
    parser_xml_delete( parser->xml );
    free( parser );
}

int parser_stats_consume( parser_stats_t *parser, void *data, size_t len )
{
    return parser_xml_consume( parser->xml, data, len );
}

static void raise_cb_if_done( parser_stats_t *parser )
{
    if( parser->bytes != 0 &&
        parser->files != 0 )
    {
        parser->cb( parser->cb_ctxt, parser->files, parser->bytes );
    }
}

void stats_xml_cb( void *ctxt, parser_xml_event_t event, char *name, char *value ) //TODO: should be static, fix tests
{
    parser_stats_t *parser = (parser_stats_t *)ctxt;


    switch( parser->state )
    {
        case state_WAITING_FOR_DIV_GENERAL:
            if( event == parser_xml_event_TAG_START &&
                !strcmp( name, "div" ) &&
                !strcmp( value, "general" ) )
            {
                parser->state = state_WAITING_FOR_SPAN_WITH_ID;
            }
            break;
        case state_WAITING_FOR_SPAN_WITH_ID:
            if( event == parser_xml_event_TAG_START &&
                !strcmp( name, "span" ) &&
                value != NULL )
            {
                if( !strcmp( value, "file-count" ) )
                {
                    parser->state = state_CONSUMING_FILE_COUNT;
                }
                else if( !strcmp( value, "total-size" ) )
                {
                    parser->state = state_CONSUMING_TOTAL_SIZE;
                }
            }
            break;
        case state_CONSUMING_FILE_COUNT:
            if( event == parser_xml_event_ATTRIBUTE &&
                !strcmp( name, "value" ) )
            {
                parser->files = atoll( value );
                raise_cb_if_done( parser );
            }
            parser->state = state_WAITING_FOR_SPAN_WITH_ID;
            break;
        case state_CONSUMING_TOTAL_SIZE:
            if( event == parser_xml_event_ATTRIBUTE &&
                !strcmp( name, "value" ) )
            {
                parser->bytes = atoll( value );
                raise_cb_if_done( parser );
            }
            parser->state = state_WAITING_FOR_SPAN_WITH_ID;
            break;
    }

    free(name);
    free(value);
}
