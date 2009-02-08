/*
 * Progress indication using curses to draw pretty things.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

/* Download tasks update this module with info about progress, which is then
 * drawn when this module sees fit (currently off an alarm).
 *
 * Some externally visible functions may be re-entered by multiple threads,
 * all associated with a different file, thus there is a lock for the hash
 * table itself (this is a mutex, as required by the uthash docs - a r/w lock
 * is insufficient). There is a bijection between threads (file downloads) and
 * hash entries. thus the progress_t entries themselves require no locking */

#include <curses.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include "uthash.h"

#include "common.h"
#include "locks.h"
#include "config.h"
#include "progress.h"
#include "trace.h"
#include "alarms.h"


TRACE_DEFINE(progress)


typedef struct
{
    char *path;
    unsigned len;
    unsigned downloaded;
    UT_hash_handle hh;
} progress_t;


static progress_t *progress_cache = NULL;
static rw_lock_t progress_cache_lock;


static void progress_bar_draw_all (void);
static void progress_bar_draw (progress_t *p,
                               unsigned line);
static const char *path_shorten (const char *path, unsigned length);
static void progress_draw_cb (void *ctxt);


WINDOW *win = NULL;


/* EXTERNS ================================================================== */

int progress_init (void)
{
    int rc = 1;


    progress_trace("progress_init()\n");
    progress_trace_indent();

    if (config_get(config_key_PROGRESS).int_val)
    {
        win = initscr();

        if (win)
        {
            noecho();
            cbreak();
            nodelay(win, TRUE);

            curs_set(0);


            rw_lock_init(&progress_cache_lock);

            alarm_schedule(0, 100000, &progress_draw_cb, NULL);

            rc = 0;
        }
    }
    else
    {
        progress_trace("progress disabled\n");

        rc = 0;
    }

    progress_trace_dedent();


    return rc;
}

void progress_finalise (void)
{
    if (config_get(config_key_PROGRESS).int_val)
    {
        endwin();

        rw_lock_destroy(&progress_cache_lock);
    }
}

void progress_update (const char *path,
                      unsigned len,
                      unsigned downloaded)
{
    progress_t *p = NULL;


    if (config_get(config_key_PROGRESS).int_val)
    {
        rw_lock_rlock(&progress_cache_lock);
        HASH_FIND(hh, progress_cache, path, (signed)strlen(path), p);
        rw_lock_runlock(&progress_cache_lock);

        if (!p)
        {
            p = (progress_t *)calloc(1, sizeof(progress_t));


            p->path = strdup(path);

            rw_lock_wlock(&progress_cache_lock);
            HASH_ADD_KEYPTR(hh, progress_cache, p->path, strlen(p->path), p);
            rw_lock_wunlock(&progress_cache_lock);
        }

        p->len = len;
        p->downloaded = downloaded;
    }
}

void progress_delete (const char *path)
{
    progress_t *p = NULL;


    if (config_get(config_key_PROGRESS).int_val)
    {
        rw_lock_rlock(&progress_cache_lock);
        HASH_FIND(hh, progress_cache, path, (signed)strlen(path), p);
        rw_lock_runlock(&progress_cache_lock);

        if (p)
        {
            rw_lock_wlock(&progress_cache_lock);
            HASH_DELETE(hh, progress_cache, p);
            rw_lock_wunlock(&progress_cache_lock);

            free(p);
        }
    }
}


/* STATICS ================================================================== */

static void progress_bar_draw_all (void)
{
    unsigned i = 0;
    progress_t *p;


    clear();

    for (p = progress_cache, i = 1; p != NULL; p = p->hh.next, i++)
    {
        progress_bar_draw(p, i);
    }

    refresh();
}

static void progress_bar_draw (progress_t *p,
                               unsigned line)
{
    char *s;
    unsigned y, line_len;
    unsigned i, n;

    getmaxyx(win, y, line_len);
    s = (char *)malloc(line_len * sizeof(char) + 1);
    n = ((double)p->downloaded / (double)p->len) * line_len;


    sprintf(s, "%s", path_shorten(p->path, line_len - 2));
    for (i = strlen(s); i < line_len; i++)
    {
        s[i] = ' ';
    }
    s[i] = '\0';

    attron(A_REVERSE);
    mvaddnstr(line, 0, s, n);
    attroff(A_REVERSE);
    mvaddnstr(line, n, s + n, line_len - n);
}

/* no need to free the result of this atm */
static const char *path_shorten (const char *path, unsigned length)
{
    size_t n = strlen(path) - MIN(strlen(path), length - 1);


    return path + n;
}

static void progress_draw_cb (void *ctxt)
{
    NOT_USED(ctxt);

    progress_bar_draw_all();

    alarm_schedule(0, 100000, &progress_draw_cb, NULL);
}
