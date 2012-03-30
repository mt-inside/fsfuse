/*
 * Safe, secure string_[builder|buffer] clone.
 *
 * Copyright (C) Matthew Turner 2011. All rights reserved.
 *
 * $Id: alarms.c 474 2010-01-29 00:01:07Z matt $
 */

#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "trace.h"
#include "string_buffer.h"


TRACE_DEFINE(string_buffer)


/* Essentially a bstring, but with a terminating NUL.
 * It is just easier to maintain the terminating-NUL invariant because e.g.
 * vsnprintf() will always write one, and peek() is then simple. */
struct _string_buffer_t
{
    size_t capacity; /* size of memory allocated for s      */
    size_t length;   /* length of string in s (not inc NUL) */
    char s[];        /* Including terminating NUL           */
};


static void string_buffer_ensure_capacity (string_buffer_t *sb, size_t cap);


string_buffer_t *string_buffer_new (void)
{
    string_buffer_t *ptr = malloc(sizeof(string_buffer_t));

    *ptr = malloc(sizeof(struct _string_buffer_t) + 1);
    (**ptr).capacity = 1;
    (**ptr).length   = 0;
    (**ptr).s[0] = '\0';

    return ptr;
}

string_buffer_t *string_buffer_from_chars (const char *string)
{
    string_buffer_t *sb = string_buffer_new();

    string_buffer_set(sb, string);

    return sb;
}

void string_buffer_delete (string_buffer_t *sb)
{
    free(*sb);
    free(sb);
}


void string_buffer_set (string_buffer_t *sb, const char *string)
{
    size_t len = strlen(string);
    string_buffer_ensure_capacity(sb, len);

    strcpy((**sb).s, string);
    (**sb).length = len;
}

void string_buffer_append (string_buffer_t *sb, const char *string)
{
    size_t len = strlen(string);
    string_buffer_ensure_capacity(sb, (**sb).length + len);

    strcpy((**sb).s + (**sb).length, string);
    (**sb).length += len;
}

/* This looks a lot like make_message in the linux vsnprintf() man page */
void string_buffer_printf (string_buffer_t *sb, const char *format, ...)
{
    va_list ap;
    int output_size;

    va_start(ap, format);
    output_size = vsnprintf((**sb).s, (*sb)->capacity, format, ap);
    va_end(ap);
    assert(output_size >= 0);

    if ((unsigned)output_size >= (*sb)->capacity)
    {
        /* return of vsnprintf is exclusive of the space needed for the NUL, but
         * ensure_capacity accounts for that */
        string_buffer_ensure_capacity(sb, output_size);

        va_start(ap, format);
        output_size = vsnprintf((**sb).s, (*sb)->capacity, format, ap);
        va_end(ap);

        assert(output_size >= 0 && (unsigned)output_size < (*sb)->capacity);
    }

    (**sb).length = output_size;
}

char *string_buffer_get (string_buffer_t *sb)
{
    char *s = malloc((**sb).length + 1);

    memcpy(s, (**sb).s, (**sb).length + 1);

    return s;
}

const char *string_buffer_peek (string_buffer_t *sb)
{
    return (**sb).s;
}

char *string_buffer_commit (string_buffer_t *sb)
{
    /* Can't just return (**sb).s because it points to half-way through a block
     * and can't be freed */
    /* sb_get() does a copy, but the other option would be to memmove() the
     * string to the front then realloc() - just as expensive */

    char *s = string_buffer_get(sb);

    free(sb);

    return s;
}

static void string_buffer_ensure_capacity (string_buffer_t *sb, size_t cap)
{
    cap += 1; /* Account for NUL character */

    if( (*sb)->capacity < cap )
    {
        /* TODO: should prolly round up to power of 2 or something */
        (*sb) = realloc( *sb, sizeof(struct _string_buffer_t) + cap );
        (*sb)->capacity = cap;
    }
}
