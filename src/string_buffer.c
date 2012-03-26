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


/* Essentially a bstring, but with a terminating NUL */
struct _string_buffer_t
{
    size_t capacity;
    size_t length;
    char s[]; /* Including terminating NUL */
};


static void string_buffer_ensure_capacity (string_buffer_t *sb, size_t cap);


string_buffer_t *string_buffer_new (void)
{
    string_buffer_t *ptr = malloc(sizeof(string_buffer_t));

    *ptr = calloc(sizeof(struct _string_buffer_t), 1);

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
    char *s = (**sb).s;

    free(sb);

    return s;
}

static void string_buffer_ensure_capacity (string_buffer_t *sb, size_t cap)
{
    if( (*sb)->capacity < cap )
    {
        (*sb) = realloc( *sb, sizeof(struct _string_buffer_t) + (cap * sizeof(char)) );
        (*sb)->capacity = cap;
    }
}
