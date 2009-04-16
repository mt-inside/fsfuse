/*
 * This file contains the programme's entry point, command line parsing, event
 * loop, etc.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <assert.h>
#include <getopt.h>
#include <string.h>

#include <fuse.h>
#include <fuse/fuse_opt.h>
#include <fuse/fuse_lowlevel.h>

#include <curl/curl.h>
#include <libxml/xmlversion.h>
#include <curses.h>

#include "common.h"
#include "config.h"
#include "alarms.h"
#include "fetcher.h"
#include "parser.h"
#include "download_thread_pool.h"
#include "direntry.h"
#if FEATURE_PROGRESS_METER
#include "progress.h"
#endif
#include "indexnode.h"

#include "fsfuse_ops/readdir.h"
#include "fsfuse_ops/read.h"
#include "fsfuse_ops/statfs.h"
#include "fsfuse_ops/others.h"


static void settings_parse (int argc, char *argv[], struct fuse_args args);
static void fsfuse_splash (void);
static void fsfuse_version (void);
static void fsfuse_usage (const char *progname);


static void *fsfuse_init (struct fuse_conn_info *conn)
{
    trce("fsfuse_init()\n");

    trace_indent();

    trce("proto_major==%u, proto_minor==%u, async_read==%u, "
         "max_write==%u, max_readahead==%u\n",
         conn->proto_major,
         conn->proto_minor,
         conn->async_read,
         conn->max_write,
         conn->max_readahead);

    trace_dedent();


    return NULL;
}

static void fsfuse_destroy (void *private_data)
{
    trce("fsfuse_destroy()\n");

    NOT_USED(private_data);
}


static struct fuse_operations fsfuse_oper = {
    &fsfuse_getattr,     /* getattr */
    &fsfuse_readlink,    /* readlink */
    NULL,                /* getdir D */
    &fsfuse_mknod,       /* mknod */
    &fsfuse_mkdir,       /* mkdir */
    &fsfuse_unlink,      /* unlink */
    &fsfuse_rmdir,       /* rmdir */
    &fsfuse_symlink,     /* symlink */
    &fsfuse_rename,      /* rename */
    &fsfuse_link,        /* link */
    &fsfuse_chmod,       /* chmod */
    &fsfuse_chown,       /* chown */
    &fsfuse_truncate,    /* truncate */
    NULL,                /* utime D */
    &fsfuse_open,        /* open */
    &fsfuse_read,        /* read */
    &fsfuse_write,       /* write */
    &fsfuse_statfs,      /* statfs */
    NULL,                /* flush */
    NULL,                /* release */
    &fsfuse_fsync,       /* fsync */
    NULL,                /* setxattr */
    NULL,                /* getxattr */
    NULL,                /* listxattr */
    NULL,                /* removexattr */
    &fsfuse_opendir,     /* opendir */
    &fsfuse_readdir,     /* readdir */
    NULL,                /* releasedir */
    &fsfuse_fsyncdir,    /* fsyncdir */
    &fsfuse_init,        /* init */
    &fsfuse_destroy,     /* destroy */
    &fsfuse_access,      /* access */
    &fsfuse_create,      /* create */
    &fsfuse_ftruncate,   /* ftruncate */
    NULL,                /* fgetattr */
    NULL,                /* lock */
    NULL,                /* utimens */
    &fsfuse_bmap         /* bmap */
};


int main(int argc, char *argv[])
{
    int rc = 1;
    struct fuse_args fuse_args;
    struct fuse_chan *ch;
    struct fuse *f;
    char my_arg[1024];


    /* Check system fuse version */
    if (fuse_version() < FUSE_USE_VERSION)
    {
        printf("%s error: fuse version %d required, only version %d available\n",
               argv[0], FUSE_USE_VERSION, fuse_version());
        exit(EXIT_FAILURE);
    }

    fsfuse_splash();

    /* Parse cmdline args */
    /* fuse *really* wants you to use its parsing mechanism. It ignores
     * fuse_args.argv[0], as if it's the program name - i.e. it's expecting a
     * copy of your argv, not a special one you've made for it. However, if you
     * give fuse_new() any arguments it doesn't understand it b0rks. This is
     * convenient, given fuse's parsing functions remove allow you to remove any
     * (non-fuse) arguments you've dealt with...
     * Anything you do give it must be malloc()d, as it all gets realloc()d and
     * free()d a lot. fuse_opt_parse() must do this implicitly.
     * In response, we copy argv[0] over to fuse's array (in case it does
     * anything with it) and fill the rest with the args we want it to see */
    fuse_args.argc = 1;
    fuse_args.argv = (char **)malloc((argc + 1) * sizeof(char *));
    fuse_args.argv[0] = strdup(argv[0]);
    fuse_args.argv[1] = NULL;
    fuse_args.allocated = 1;
    settings_parse(argc, argv, fuse_args);

    /* Here we add mount options to give fuse.
     * These seem to be normal mount options, plus some fuse-specific ones.
     * Presumably we can also read (and filter out) any fsfuse-specific ones
     * from our command line.
     * Apparently, default options are "nodev,nosuid". Others need to be added.
     * It looks like fuse adds "user=<foo>".
     */
    sprintf(my_arg, "-ofsname=" FSFUSE_NAME);
    fuse_opt_add_arg(&fuse_args, my_arg);

    sprintf(my_arg, "-osubtype=" FSFUSE_NAME "-" FSFUSE_VERSION);
    fuse_opt_add_arg(&fuse_args, my_arg);

    sprintf(my_arg, "-oallow_other");
    fuse_opt_add_arg(&fuse_args, my_arg);

    sprintf(my_arg, "-oro");
    fuse_opt_add_arg(&fuse_args, my_arg);


    if (config_get(config_key_DEBUG).int_val)
    {
        config_set_int(config_key_FOREGROUND, 1);
        trace_on();
    }

    /* Inits */
    if (trace_init()       ||
        config_init()      ||
        alarms_init()      ||
        fetcher_init()     ||
        parser_init()      ||
        direntry_init()    ||
#if FEATURE_PROGRESS_METER
        progress_init()    ||
#endif
        thread_pool_init()    )
    {
        printf("%s error: initialisation failed\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Check indexnode version */
    if (indexnode_find())
    {
        printf("%s error: indexnode find error (this is not a simple indexnode not found)\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if (indexnode_version() > PROTO_GREATEST)
    {
        printf("%s error: indexnode reports to be version %f, only versions <= %f are supported\n",
               argv[0], indexnode_version(), PROTO_GREATEST);
        exit(EXIT_FAILURE);
    }

    /* Hand over to fuse */
    if ((ch = fuse_mount(config_get(config_key_MOUNTPOINT).str_val, &fuse_args)))
    {
        if ((f = fuse_new(ch, &fuse_args, &fsfuse_oper, sizeof(fsfuse_oper), NULL)))
        {
            /* Setup */
            fuse_set_signal_handlers(fuse_get_session(f));
            fuse_daemonize(config_get(config_key_FOREGROUND).int_val);

            /* Go! */
            printf("handing over to fuse_loop*()...\n");
            if (config_get(config_key_SINGLE_THREADED).int_val)
            {
                rc = fuse_loop(f);
            }
            else
            {
                rc = fuse_loop_mt(f);
            }

            /* Teardown */
            fuse_remove_signal_handlers(fuse_get_session(f));
            fuse_unmount(config_get(config_key_MOUNTPOINT).str_val, ch);
            fuse_destroy(f);
        }
    }
    fuse_opt_free_args(&fuse_args);


    /* finalisations */
    thread_pool_finalise();
#if FEATURE_PROGRESS_METER
    progress_finalise();
#endif
    direntry_finalise();
    parser_finalise();
    fetcher_finalise();
    alarms_finalise();
    config_finalise();
    trace_finalise();


    exit(rc);
}

static void settings_parse (int argc, char *argv[], struct fuse_args args)
{
    char my_arg[1024];
    int c, option_index = 0;
    char *mountpoint;
    struct option long_options[] = {
        {"debug",          no_argument,       NULL, 'd'},
        {"foreground",     no_argument,       NULL, 'f'},
        {"singlethreaded", no_argument,       NULL, 's'},
        {"progress",       no_argument,       NULL, 'p'},
        {"trace",          required_argument, NULL, 't'},
        {"help",           no_argument,       NULL, 'h'},
        {"version",        no_argument,       NULL, 'v'},
        {0, 0, 0, 0}
    };

    if (argc < 3)
    {
        fsfuse_usage(argv[0]);
        exit(1);
    }
    /* standard mount options:
     * - argv[1] is "what" to mount - ignored
     * - argv[2] is "where" to mount - mountpoint */
    mountpoint = (char *)malloc(PATH_MAX * sizeof(char *));
    if (realpath(argv[2], mountpoint) == NULL)
    {
        fprintf(stderr,
                "fsfuse: bad mount point `%s': %s\n",
                argv[optind + 1], strerror(errno));
        exit(1);
    }
    config_set_string(config_key_MOUNTPOINT, mountpoint);

    argv += 2;
    argc -= 2;

    while (1)
    {
        c = getopt_long(argc, argv, "dfspt:hvo", long_options, &option_index);
        if (c == -1) break;

        switch (c)
        {
            case 'd':
                /* debug */
                config_set_int(config_key_DEBUG, 1);
                sprintf(my_arg, "-d");
                fuse_opt_add_arg(&args, my_arg);
                break;

            case 'f':
                /* foreground */
                config_set_int(config_key_FOREGROUND, 1);
                break;

            case 's':
                /* single threaded */
                config_set_int(config_key_SINGLE_THREADED, 1);
                break;

            case 'p':
                /* progress meter */
                config_set_int(config_key_PROGRESS, 1);
                break;

            case 't':
                /* trace */
#define TRACE_ARG(aREA)                          \
                if (!strcmp(optarg, #aREA))      \
                {                                \
                    aREA##_trace_on();           \
                }

                TRACE_ARG(alarms)
                TRACE_ARG(direntry)
                TRACE_ARG(dtp)
                TRACE_ARG(fetcher)
                TRACE_ARG(method)
                TRACE_ARG(parser)
#if FEATURE_PROGRESS_METER
                TRACE_ARG(progress)
#endif
                TRACE_ARG(read)

                break;

            case 'v':
                fsfuse_version();
                exit(0);
                break;

            case 'h':
            case '?':
                fsfuse_usage(argv[0]);
                exit(0);
                break;

            case 'o':
                /* generated by mount - ignore for now */
                break;

            default:
                printf("getopt returned character %c\n", c);
                assert(0);
        }
    }

    if (optind < argc)
    {
        printf("non-option argv elements: ");
        while (optind < argc)
        {
            printf("%s ", argv[optind++]);
        }
        printf("\n");
    }
}

static void fsfuse_splash (void)
{
    printf("%s - FragSoc Filesystem in USErspace\n"
           "Version %s - %s\n"
           "%s\n"
           "\n",
           FSFUSE_NAME,
           FSFUSE_VERSION,
           FSFUSE_DATE,
           FSFUSE_COPYRIGHT
          );
}

static void fsfuse_version (void)
{
    printf("Using FUSE features up to version %d (system library supportes up to %d)\n"
           "Using %s\n"
           "Using libxml2 %s\n"
           "Using %s\n"
           "\n",
           FUSE_USE_VERSION,
           fuse_version(),
           curl_version(),
           LIBXML_DOTTED_VERSION, /* This is the version of the headers on this machine, but xmlParserVersion is ugly */
           curses_version()
          );
}

static void fsfuse_usage (const char *progname)
{
    printf("usage: %s device mountpoint [-o option[,...]]\n"
           "    device: device to mount - ignored\n"
           "    mountpoint: directory over which to mount\n"
           "\n",
           progname);
}
