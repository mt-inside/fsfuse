/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * API for debug output system.
 */

#ifndef _INCLUDED_TRACE_H
#define _INCLUDED_TRACE_H

#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>


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


#if DEBUG

/* Unconditional trace */
#define trce(...) uncond_real_trace(__VA_ARGS__) /* Stupid ncurses. l2namespace */
#define trace_np(...) uncond_real_trace_np(__VA_ARGS__)
#define trace_indent uncond_real_trace_indent
#define trace_dedent uncond_real_trace_dedent

/* Area trace indirection macros */
#define TRACE(area,...)    area##_real_trace(__VA_ARGS__)
#define TRACE_NP(area,...) area##_real_trace_np(__VA_ARGS__)
#define TRACE_INDENT(area) area##_real_trace_indent()
#define TRACE_DEDENT(area) area##_real_trace_dedent()

#define TRACE_DECLARE(area)                                               \
    extern int  area##_trace_active;                                      \
    extern void area##_real_trace (const char *fmt, ...);                 \
    extern void area##_real_trace_np (const char *fmt, ...);              \
    extern void area##_trace_on (void);                                   \
    extern void area##_trace_off (void);                                  \
    extern void area##_real_trace_indent (void);                          \
    extern void area##_real_trace_dedent (void);

#define TRACE_DEFINE(area)                                                \
                                                                          \
    int area##_trace_active = 0;                                          \
                                                                          \
    void area##_real_trace (const char *fmt, ...)                         \
    {                                                                     \
        va_list ap;                                                       \
                                                                          \
                                                                          \
        va_start(ap, fmt);                                                \
        if (area##_trace_active)                                          \
        {                                                                 \
            active_emitter.emitter(0, #area, fmt, ap);                    \
        }                                                                 \
        va_end(ap);                                                       \
    }                                                                     \
                                                                          \
    void area##_real_trace_np (const char *fmt, ...)                      \
    {                                                                     \
        va_list ap;                                                       \
                                                                          \
                                                                          \
        va_start(ap, fmt);                                                \
        if (area##_trace_active)                                          \
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
    void area##_real_trace_indent (void)                                  \
    {                                                                     \
        if (area##_trace_active)                                          \
        {                                                                 \
            pthread_mutex_lock(&trace_mutex);                             \
            trace_dent += 2;                                              \
            pthread_mutex_unlock(&trace_mutex);                           \
        }                                                                 \
    }                                                                     \
                                                                          \
    void area##_real_trace_dedent (void)                                  \
    {                                                                     \
        if (area##_trace_active)                                          \
        {                                                                 \
            pthread_mutex_lock(&trace_mutex);                             \
            if (trace_dent) trace_dent -= 2;                              \
            pthread_mutex_unlock(&trace_mutex);                           \
        }                                                                 \
    }

#else /* DEBUG */

/* Unconditional trace */
#define trce(...)
#define trace_np(...)
#define trace_indent()
#define trace_dedent()

/* Area trace indirection macros */
#define TRACE(area,...)
#define TRACE_NP(area,...)
#define TRACE_INDENT(area)
#define TRACE_DEDENT(area)

#define TRACE_DECLARE(area)
#define TRACE_DEFINE(area)

#endif /* DEBUG */


TRACE_DECLARE(uncond)


extern pthread_mutex_t trace_mutex;
extern unsigned trace_dent;


extern int trace_init (void);
extern void trace_finalise (void);


/* "console" printing support */
extern void trace_error (const char *fmt, ...);
extern void trace_warn  (const char *fmt, ...);
extern void trace_info  (const char *fmt, ...);

#endif /* _INCLUDED_TRACE_H */
