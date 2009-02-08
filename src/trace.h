/*
 * API for debug output system.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#ifndef _INCLUDED_TRACE_H
#define _INCLUDED_TRACE_H

#include <stdio.h>
#include <stdarg.h>


typedef enum
{
    emitter_flag_NO_PREFIX = (1U << 0)
} emitter_flags_t;

typedef struct
{
    int  (*emitter_init) (void);
    void (*emitter_finalise) (void);
    void (*emitter) (emitter_flags_t flags,
                     const char *prefix,
                     const char *fmt,
                     va_list ap             );
} emitter_t;

#define active_emitter emitter_logfile
extern emitter_t active_emitter;


#define TRACE_DECLARE(area)                                               \
    extern int area##_trace_active;                                       \
    extern void area##_trace    (const char *fmt, ...);                   \
    extern void area##_trace_np (const char *fmt, ...);                   \
    extern void area##_trace_on (void);                                   \
    extern void area##_trace_off (void);                                  \
    extern void area##_trace_indent (void);                               \
    extern void area##_trace_dedent (void);

#define TRACE_DEFINE(area)                                                \
                                                                          \
    int area##_trace_active = 0;                                          \
                                                                          \
                                                                          \
    void area##_trace    (const char *fmt, ...)                           \
    {                                                                     \
        va_list ap;                                                       \
                                                                          \
                                                                          \
        va_start(ap, fmt);                                                \
        if (trace_active && area##_trace_active)                          \
        {                                                                 \
            active_emitter.emitter(0, #area, fmt, ap);                    \
        }                                                                 \
        va_end(ap);                                                       \
    }                                                                     \
                                                                          \
    void area##_trace_np (const char *fmt, ...)                           \
    {                                                                     \
        va_list ap;                                                       \
                                                                          \
                                                                          \
        va_start(ap, fmt);                                                \
        if (trace_active && area##_trace_active)                          \
        {                                                                 \
            active_emitter.emitter(emitter_flag_NO_PREFIX, #area, fmt, ap); \
        }                                                                 \
        va_end(ap);                                                       \
    }                                                                     \
                                                                          \
    void area##_trace_on (void)                                           \
    {                                                                     \
        area##_trace_active = 1;                                          \
    }                                                                     \
    void area##_trace_off (void)                                          \
    {                                                                     \
        area##_trace_active = 0;                                          \
    }                                                                     \
                                                                          \
void area##_trace_indent (void)                                           \
{                                                                         \
    if (area##_trace_active)                                              \
    {                                                                     \
        trace_dent += 2;                                                  \
    }                                                                     \
}                                                                         \
void area##_trace_dedent (void)                                           \
{                                                                         \
    if (area##_trace_active)                                              \
    {                                                                     \
        if (trace_dent) trace_dent -= 2;                                  \
    }                                                                     \
}


extern int trace_init (void);
extern void trace_finalise (void);

extern void trace_on  (void);
extern void trace_off (void);


extern unsigned trace_dent;
extern int trace_active;


/* now, a half-arsed manual implimentation, to create unconditional trace */
extern void trce (const char *fmt, ...); /* stupid curses. l2namespace. */
extern void trace_indent (void);
extern void trace_dedent (void);

#endif /* _INCLUDED_TRACE_H */
