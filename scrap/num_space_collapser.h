
typedef struct _num_space_collapser_t num_space_collapser_t;


extern num_space_collapser_t *num_space_collapser_new (void);
extern void num_space_collapser_delete (num_space_collapser_t *nsc);
extern unsigned num_space_collapser_get (num_space_collapser_t *nsc, unsigned key);
