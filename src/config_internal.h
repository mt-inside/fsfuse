typedef enum
{
    config_item_type_STRING,
    config_item_type_NUMBER
} config_item_type_t;

typedef struct
{
    void *symbol;
    config_item_type_t type;
    char *xpath;
} config_item;

extern config_item config_items[];