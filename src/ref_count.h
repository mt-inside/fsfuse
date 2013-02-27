/*
 * Copyright (C) 2008-2013 Matthew Turner. Distributed under the GPL v3.
 *
 * Reference-counting class.
 *
 */

#ifndef _INCLUDED_REF_COUNT_H
#define _INCLUDED_REF_COUNT_H

typedef struct _ref_count_t ref_count_t;


extern ref_count_t *ref_count_new( );
extern void ref_count_delete( ref_count_t *refc );

extern unsigned ref_count_inc( ref_count_t *refc /* logger, maybe null */ );
extern unsigned ref_count_dec( ref_count_t *refc );

#endif /* _INCLUDED_REF_COUNT_H */
