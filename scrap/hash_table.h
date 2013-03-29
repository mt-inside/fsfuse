/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * Simple hash table API.
 */

#ifndef _INCLUDED_HASH_TABLE_H
#define _INCLUDED_HASH_TABLE_H

typedef struct _hash_table_t hash_table_t;
typedef struct _hash_table_iterator_t hash_table_iterator_t;


extern hash_table_t *hash_table_new (double max_load, double min_load);
extern void hash_table_delete (hash_table_t *tbl);
extern void hash_table_add (hash_table_t *tbl, const char *key, void *data);
extern void *hash_table_find (hash_table_t *tbl, const char *key);
extern int hash_table_del (hash_table_t *tbl, const char *key);

#if DEBUG
extern void hash_table_dump_dot (hash_table_t *tbl);
#endif

extern hash_table_iterator_t *hash_table_iterator_new (hash_table_t *tbl);
extern void hash_table_iterator_delete (hash_table_iterator_t *iter);
extern const char *hash_table_iterator_current_key (hash_table_iterator_t *iter);
extern void *hash_table_iterator_current_data (hash_table_iterator_t *iter);
extern int hash_table_iterator_next (hash_table_iterator_t *iter);
extern int hash_table_iterator_at_end (hash_table_iterator_t *iter);

#endif /* _INCLUDED_HASH_TABLE_H */
