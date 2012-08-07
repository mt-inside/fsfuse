/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * Safe, secure string_[builder|buffer] clone.
 */

#ifndef _INCLUDED_STRING_BUFFER_H
#define _INCLUDED_STRING_BUFFER_H

#include "trace.h"


TRACE_DECLARE(string_buffer)
#define string_buffer_trace(...) TRACE(string_buffer,__VA_ARGS__)
#define string_buffer_trace_indent() TRACE_INDENT(string_buffer)
#define string_buffer_trace_dedent() TRACE_DEDENT(string_buffer)


/* We hide a layer of indirection here so that we have double-pointers and can
 * realloc() them. This is actually pointless because we may as well have a
 * struct with an external char *, and we'd have the same number of redirects
 */
typedef struct _string_buffer_t *string_buffer_t;


/* Not ref-counted, one owner only */
extern string_buffer_t *string_buffer_new (void);
extern string_buffer_t *string_buffer_from_chars (const char *string);
extern void string_buffer_delete (string_buffer_t *sb);

extern void string_buffer_set (string_buffer_t *sb, const char *string);
extern void string_buffer_append (string_buffer_t *sb, const char *string);
extern void string_buffer_printf (string_buffer_t *sb, const char *format, ...);

extern char *string_buffer_get (string_buffer_t *sb);
extern const char *string_buffer_peek (string_buffer_t *sb);
extern char *string_buffer_commit (string_buffer_t *sb);

#endif /* _INCLUDED_STRING_BUFFER_H */
