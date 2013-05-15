
typedef struct _kvp_t kvp_t;


extern kvp_t *kvp_new( int key, void *value );
extern void kvp_delete( kvp_t *kvp );

extern int kvp_key( kvp_t *kvp );
extern void *kvp_value( kvp_t *kvp );
