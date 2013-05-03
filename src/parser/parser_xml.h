
typedef struct _parser_xml_t parser_xml_t;

typedef enum
{
    parser_xml_event_TAG_START,
    parser_xml_event_TEXT,
    parser_xml_event_TAG_END
} parser_xml_event_t;

typedef void (*parser_xml_cb_t)( void *ctxt, parser_xml_event_t event, char *text );


extern parser_xml_t *parser_xml_new(
    parser_xml_cb_t cb,
    void *cb_ctxt
);
extern void parser_xml_delete( parser_xml_t *xml );

extern int parser_xml_consume( parser_xml_t *xml, void *data, size_t len );
