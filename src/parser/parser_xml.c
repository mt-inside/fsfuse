#include "common.h"

#include <stdlib.h>
#include <string.h>
#include <libxml/SAX.h>

#include "parser_xml.h"


struct _parser_xml_t
{
    parser_xml_cb_t cb;
    void *cb_ctxt;
    xmlSAXHandler *sax;
    xmlParserCtxtPtr sax_ctxt;
};


int parser_xml_init( void )
{
    LIBXML_TEST_VERSION

#ifndef LIBXML_PUSH_ENABLED
#error libxml not comiled with push mode support
#endif

    xmlInitParser( );


    return 0;
}

void parser_xml_finalise( void )
{
    xmlCleanupParser();
}

static void on_start_element_ns(
    void *ctxt,
    const xmlChar *localname,
    const xmlChar *prefix,
    const xmlChar *URI,
    int nb_namespaces,
    const xmlChar **namespaces,
    int nb_attributes,
    int nb_defaulted,
    const xmlChar **attributes
)
{
    parser_xml_t *xml = (parser_xml_t *)ctxt;

    xml->cb( xml->cb_ctxt, parser_xml_event_TAG_START, strdup( (char *)localname ) );

    NOT_USED( prefix );
    NOT_USED( URI );
    NOT_USED( nb_namespaces );
    NOT_USED( namespaces );
    NOT_USED( nb_attributes );
    NOT_USED( nb_defaulted );
    NOT_USED( attributes );
}

static void on_end_element_ns(
    void *ctxt,
    const xmlChar *localname,
    const xmlChar *prefix,
    const xmlChar *URI
)
{
    parser_xml_t *xml = (parser_xml_t *)ctxt;

    xml->cb( xml->cb_ctxt, parser_xml_event_TAG_END, strdup( (char *)localname ) );

    NOT_USED( prefix );
    NOT_USED( URI );
}

static void on_characters(
    void *ctxt,
    const xmlChar *ch,
    int len
)
{
    parser_xml_t *xml = (parser_xml_t *)ctxt;
    char *chars;

    /* Don't send character runs that are /only/ whitespace as this is probably
     * formatting in the original document. Alas libxml2 doesn't seem to want to
     * do this for us. There are options to stop the DOM builder adding blank
     * #text nodes, but nothing to stop SAX raising them. */
    if( (signed)strspn( (char *)ch, " \t\n\r" ) != len )
    {
        chars = malloc( len + 1 );
        strncpy( chars, (char *)ch, len );
        chars[ len ] = '\0';

        xml->cb( xml->cb_ctxt, parser_xml_event_TEXT, chars );
    }
}


parser_xml_t *parser_xml_new(
    parser_xml_cb_t cb,
    void *cb_ctxt
)
{
    parser_xml_t *xml = malloc( sizeof(*xml) );


    xml->cb = cb;
    xml->cb_ctxt = cb_ctxt;

    xml->sax = calloc( 1, sizeof(*xml->sax) );
    xml->sax->initialized    = XML_SAX2_MAGIC;
    xml->sax->startElementNs = &on_start_element_ns;
    xml->sax->endElementNs   = &on_end_element_ns;
    xml->sax->characters     = &on_characters;

    xml->sax_ctxt = xmlCreatePushParserCtxt(
        xml->sax,
        xml, /* user context */
        NULL, 0, /* initial chunk (&len) used for encoding detection */
        NULL /* file for external entities */
    );


    return xml;
}

void parser_xml_delete( parser_xml_t *xml )
{
    xmlParseChunk( xml->sax_ctxt, NULL, 0, 1 );
    xmlFreeParserCtxt( xml->sax_ctxt );
    free( xml->sax );
    free( xml );
}

int parser_xml_consume( parser_xml_t *xml, void *data, size_t len )
{
    return xmlParseChunk( xml->sax_ctxt, data, len, 0 );
}
