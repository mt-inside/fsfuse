/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Debug output system.
 */

#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "common.h"
#include "trace.h"
#include "utils.h"


TRACE_DEFINE(uncond)


#define EMITTER_DECLARE(kind)                                                 \
static int emitter_##kind##_init (void);                                      \
static void emitter_##kind##_finalise (void);                                 \
static void emitter_##kind##_emitter (emitter_flags_t flags,                  \
                                      const char *prefix,                     \
                                      const char *fmt,                        \
                                      va_list ap             );               \
emitter_t emitter_##kind =                                                    \
{                                                                             \
    &emitter_##kind##_init,                                                   \
    &emitter_##kind##_finalise,                                               \
    &emitter_##kind##_emitter                                                 \
};


EMITTER_DECLARE(printf)

EMITTER_DECLARE(logfile)
FILE *log_file;


static char *trace_get_tabs (void);


pthread_mutex_t trace_mutex;
unsigned trace_dent = 0;


int trace_init (void)
{
    pthread_mutex_init(&trace_mutex, NULL);


    return active_emitter.emitter_init();
}
void trace_finalise (void)
{
    active_emitter.emitter_finalise();

    pthread_mutex_destroy(&trace_mutex);
}


#define TAB_MAX 1024
static char *trace_get_tabs (void)
{
    unsigned i;
    char *tabs = (char *)malloc(TAB_MAX * sizeof(char));
    char *c = tabs;


    trace_dent = MIN(trace_dent, TAB_MAX);
    *c = '\0';
    for (i = 0; i < trace_dent; i++)
    {
        c += sprintf(c, " ");
    }


    return tabs;
}

/*
 * printf() emitter
 */
static int emitter_printf_init (void)
{
    DO_NOTHING;

    return 0;
}
static void emitter_printf_finalise (void)
{
    DO_NOTHING;
}
static void emitter_printf_emitter (emitter_flags_t flags,
                                    const char *prefix,
                                    const char *fmt,
                                    va_list ap             )
{
    char *tabs;


    NOT_USED(prefix);

    pthread_mutex_lock(&trace_mutex);

    tabs = trace_get_tabs();

    if (!(flags & emitter_flag_NO_PREFIX))
    {
        printf("%s[%u] ",
                tabs,
                fsfuse_get_thread_index());
    }
    vprintf(fmt, ap);

    free(tabs);

    pthread_mutex_unlock(&trace_mutex);
}

/*
 * log file emitter
 */
static int emitter_logfile_init (void)
{
    log_file = fopen("fsfuse.log", "w+");


    return (log_file == NULL);
}
static void emitter_logfile_finalise (void)
{
    fclose(log_file);
}
static void emitter_logfile_emitter (emitter_flags_t flags,
                                     const char *prefix,
                                     const char *fmt,
                                     va_list ap             )
{
    char *tabs;


    NOT_USED(prefix);

    pthread_mutex_lock(&trace_mutex);

    tabs = trace_get_tabs();

    if (!(flags & emitter_flag_NO_PREFIX))
    {
        fprintf(log_file,
                "%s[%u] ",
                tabs,
                fsfuse_get_thread_index());
    }
    vfprintf(log_file, fmt, ap);

    fflush(log_file);

    free(tabs);

    pthread_mutex_unlock(&trace_mutex);
}


/* Release build "console" printing support */
static void trace_print (const char *prefix, const char *fmt, va_list ap)
{
    fprintf(stderr, "%s | ", prefix);
    vfprintf(stderr, fmt, ap);
}

void trace_error (const char *fmt, ...)
{
    va_list ap;


    va_start(ap, fmt);
    trace_print("ERROR", fmt, ap);
    va_end(ap);

    exit(1);
}

void trace_warn (const char *fmt, ...)
{
    va_list ap;


    va_start(ap, fmt);
    trace_print("WARN", fmt, ap);
    va_end(ap);
}

void trace_info (const char *fmt, ...)
{
    va_list ap;


    va_start(ap, fmt);
    trace_print("INFO", fmt, ap);
    va_end(ap);
}
