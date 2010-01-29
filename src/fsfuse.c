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

#include <sys/utsname.h>
#include <curl/curl.h>
#include <libxml/xmlversion.h>
#include <curses.h>

#include "common.h"
#include "buildnumber.h"
#include "config.h"
#include "alarms.h"
#include "fetcher.h"
#include "parser.h"
#include "download_thread_pool.h"
#include "direntry.h"
#if FEATURE_DIRENTRY_CACHE
#include "direntry_cache.h"
#endif
#if FEATURE_PROGRESS_METER
#include "progress.h"
#endif
#include "peerstats.h"
#include "indexnode.h"
#include "localei.h"

#include "fsfuse_ops/fsfuse_ops.h"


typedef enum
{
    start_action_MOUNT,
    start_action_VERSION,
    start_action_USAGE
} start_action_t;

typedef struct
{
    char *real;
    char *raw;
    char *error;
} mountpoint_t;


static char *progname = NULL;
static mountpoint_t mountpoint;


static void settings_get_config_file              (int argc, char *argv[]);
static start_action_t settings_parse_command_line (int argc, char *argv[]);
static void fuse_args_set (struct fuse_args *fuse_args);
static void fsfuse_splash (void);
static void fsfuse_versions (void);
static void fsfuse_usage (void);


int main(int argc, char *argv[])
{
    int rc = EXIT_FAILURE;
    char **myargv = malloc(argc * sizeof(char *));
    start_action_t sa;
    struct fuse_args fuse_args;
    struct fuse_chan *ch;
    struct fuse *f;


    progname = argv[0];

    fsfuse_splash();

    xmlInitParser();

    /* Check system fuse version */
    if (fuse_version() < FUSE_USE_VERSION)
    {
        trace_warn("%s error: fuse version %d required, only version %d available\n",
                   progname, FUSE_USE_VERSION, fuse_version());
        goto pre_init_bail;
    }


    /* Perfunctory parse of the command line just to find the name of the config
     * file, if specified. This has to be done first, so we can then load
     * settings from it, /before/ parsing the command line proper, over-writing
     * any config settings with higher-priority command line values */
    memcpy(myargv, argv, argc * sizeof(char *));
    settings_get_config_file(argc, myargv);
    free(myargv);


    config_init();
    config_read();


    /* Parse cmdline args */
    /* fuse *really* wants you to use its parsing mechanism. It ignores
     * fuse_args.argv[0], as if it's the program name - i.e. it's expecting a
     * copy of your argv, not a special one you've made for it. However, if you
     * give fuse_new() any arguments it doesn't understand it b0rks. This is
     * convenient, given fuse's parsing functions allow you to remove any
     * (non-fuse) arguments you've dealt with...
     * Anything you do give it must be malloc()d, as it all gets realloc()d and
     * free()d a lot. fuse_opt_parse() must do this implicitly.
     * In response, we copy argv[0] over to fuse's array (in case it does
     * anything with it) and fill the rest with the args we want it to see */
    sa = settings_parse_command_line(argc, argv);


    fuse_args_set(&fuse_args);


    switch (sa)
    {
        case start_action_MOUNT:
            if (!mountpoint.real)
            {
                trace_error("bad or missing mount point \"%s\": %s.\n", mountpoint.raw, mountpoint.error);
                fsfuse_usage();

                goto pre_init_bail;
            }
            break;

        case start_action_VERSION:
            fsfuse_versions();

            goto pre_init_bail;
            break;

        case start_action_USAGE:
            fsfuse_usage();

            goto pre_init_bail;
            break;
    }



    /* Inits */
    if (trace_init()          ||
        common_init()         ||
        locale_init()         ||
        alarms_init()         ||
        indexnode_init()      ||
        fetcher_init()        ||
        parser_init()         ||
        direntry_init()       ||
#if FEATURE_DIRENTRY_CACHE
        direntry_cache_init() ||
#endif
#if FEATURE_PROGRESS_METER
        progress_init()       ||
#endif
        peerstats_init()      ||
        thread_pool_init()    )
    {
        trace_error("initialisation failed\n");
        goto pre_init_bail;
    }

    /* Check indexnode version */
    if (indexnode_find())
    {
        trace_error("indexnode find error (this is not a simple indexnode not found)\n");
        goto bail;
    }
    if (compare_dotted_version(PROTO_MINIMUM, indexnode_version()) > 0 ||
        compare_dotted_version(indexnode_version(), PROTO_MAXIMUM) < 0)
    {
        trace_error("indexnode reports to be version %s, only versions %s <= x <= %s are supported\n",
                    indexnode_version(), PROTO_MINIMUM, PROTO_MAXIMUM);
        goto bail;
    }

    /* Hand over to fuse */
    if ((ch = fuse_mount(mountpoint.real, &fuse_args)))
    {
        if ((f = fuse_new(ch, &fuse_args, &fsfuse_oper, sizeof(fsfuse_oper), NULL)))
        {
            /* Setup */
            fuse_set_signal_handlers(fuse_get_session(f));
            fuse_daemonize(config_proc_fg);

            /* Go! */
            if (config_proc_singlethread)
            {
                rc = fuse_loop(f);
            }
            else
            {
                rc = fuse_loop_mt(f);
            }

            /* Teardown */
            fuse_remove_signal_handlers(fuse_get_session(f));
            fuse_unmount(mountpoint.real, ch);
            fuse_destroy(f);
        }
    }


bail:
    /* finalisations */
    thread_pool_finalise();
    peerstats_finalise();
#if FEATURE_PROGRESS_METER
    progress_finalise();
#endif
#if FEATURE_DIRENTRY_CACHE
    direntry_cache_finalise();
#endif
    direntry_finalise();
    parser_finalise();
    fetcher_finalise();
    alarms_finalise();
    indexnode_finalise();
    locale_finalise();
    common_finalise();
    trace_finalise();

pre_init_bail:
    config_finalise();
    xmlCleanupParser();
    fuse_opt_free_args(&fuse_args);
    if (mountpoint.real) free(mountpoint.real);
    if (mountpoint.error) free(mountpoint.error);


    exit(rc);
}

static void settings_get_config_file (int argc, char *argv[])
{
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"config",         required_argument, NULL, 'c'},
        {0, 0, 0, 0}
    };


    optind = 0;
    opterr = 0;

    while (1)
    {
        c = getopt_long(argc, argv, "c:", long_options, &option_index);
        if (c == -1) break;

        switch (c)
        {
            case 'c':
                /* config file */
                config_path_set(strdup(optarg));
                break;

            case '?':
                break;

            default:
                assert(0);
        }
    }
}

static start_action_t settings_parse_command_line (int argc, char *argv[])
{
    start_action_t rc = start_action_MOUNT;
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"config",         required_argument, NULL, 'c'},
        {"debug",          no_argument,       NULL, 'd'},
        {"foreground",     no_argument,       NULL, 'f'},
        {"singlethreaded", no_argument,       NULL, 's'},
        {"progress",       no_argument,       NULL, 'p'},
        {"trace",          required_argument, NULL, 't'},
        {"help",           no_argument,       NULL, 'h'},
        {"version",        no_argument,       NULL, 'v'},
        {0, 0, 0, 0}
    };


    optind = 0;
    opterr = 1;

    while (1)
    {
        c = getopt_long(argc, argv, "c:dfspt:h?vo", long_options, &option_index);
        if (c == -1) break;

        switch (c)
        {
            case 'c':
                /* config file - dealt with in settings_get_config_file() */
                break;

            case 'd':
                /* debug */
                config_proc_debug = 1;
                /* fall through; d => f */

            case 'f':
                /* foreground */
                config_proc_fg = 1;
                break;

            case 's':
                /* single threaded */
                config_proc_singlethread = 1;
                break;

            case 'p':
                /* progress meter */
                config_option_progress = 1;
                break;

            case 't':
                /* trace */
#if DEBUG
#define TRACE_ARG(aREA)                          \
                if (!strcmp(optarg, #aREA))      \
                {                                \
                    aREA##_trace_on();           \
                }
#else
#define TRACE_ARG(aREA)
#endif

                TRACE_ARG(alarms)
                TRACE_ARG(direntry)
#if FEATURE_DIRENTRY_CACHE
                TRACE_ARG(direntry_cache)
#endif
                TRACE_ARG(dtp)
                TRACE_ARG(fetcher)
                TRACE_ARG(method)
                TRACE_ARG(parser)
                TRACE_ARG(peerstats)
#if FEATURE_PROGRESS_METER
                TRACE_ARG(progress)
#endif
                TRACE_ARG(read)
                TRACE_ARG(locale)

                break;

            case 'v':
                rc = start_action_VERSION;
                break;

            case 'h':
            case '?':
                rc = start_action_USAGE;
                break;

            case 'o':
                /* generated by mount - ignore for now */
                break;

            default:
                assert(0);
        }
    }

    if (argc - optind >= 2)
    {
        /* standard mount options:
         * - argv[1] is "what" to mount - ignored
         * - argv[2] is "where" to mount - mountpoint */
        optind++;
        errno = 0;
        mountpoint.raw = argv[optind];
        mountpoint.real = realpath(argv[optind], NULL);
        if (!mountpoint.real) mountpoint.error = strdup(strerror(errno));
        optind++;

        if (optind < argc)
        {
            trace_warn("Unknown command line arguments.\n");

            printf("non-option argv elements: ");
            while (optind < argc)
            {
                printf("%s ", argv[optind++]);
            }
            printf("\n");
        }
    }



    return rc;
}

static void fuse_args_set (struct fuse_args *fuse_args)
{
    char my_arg[1024];
    fuse_args->argc = 0;
    fuse_args->argv = NULL;
    fuse_opt_add_arg(fuse_args, progname);


    /* Here we add mount options to give fuse.
     * These seem to be normal mount options, plus some fuse-specific ones.
     * Presumably we can also read (and filter out) any fsfuse-specific ones
     * from our command line.
     * Apparently, default options are "nodev,nosuid". Others need to be added.
     * It looks like fuse adds "user=<foo>".
     */
    if (config_proc_debug)
    {
        sprintf(my_arg, "-d");
        fuse_opt_add_arg(fuse_args, my_arg);
    }

    sprintf(my_arg, "-ofsname=" FSFUSE_NAME);
    fuse_opt_add_arg(fuse_args, my_arg);

    sprintf(my_arg, "-osubtype=" FSFUSE_NAME "-" FSFUSE_VERSION);
    fuse_opt_add_arg(fuse_args, my_arg);

    sprintf(my_arg, "-oallow_other");
    fuse_opt_add_arg(fuse_args, my_arg);

    sprintf(my_arg, "-oro");
    fuse_opt_add_arg(fuse_args, my_arg);
}

static void fsfuse_splash (void)
{
    printf("%s - FragSoc Filesystem in USErspace\n"
           "Version %s - %s (subversion revision %u)\n"
           "%s\n"
           "\n",
           FSFUSE_NAME,
           FSFUSE_VERSION,
           FSFUSE_DATE,
           svn_rev,
           FSFUSE_COPYRIGHT
          );
}

static void fsfuse_versions (void)
{
    struct utsname un;


    uname(&un);

    printf("Using kernel %s %s\n"
           "Using libfuse version %d (built against %d, using API version %d)\n"
           "Using %s\n"
           "Using libxml2 %s\n",
           un.sysname,
           un.release,
           fuse_version(), FUSE_VERSION, FUSE_USE_VERSION,
           curl_version(),
           LIBXML_DOTTED_VERSION /* This is the version of the headers on this machine, but xmlParserVersion is ugly */
          );

#if FEATURE_PROGRESS_METER
    printf("Using %s\n",
           curses_version()
          );
#endif

    printf("\n");
}

static void fsfuse_usage (void)
{
    printf("usage: %s device mountpoint [-o option[,...]]\n"
           "    device: device to mount - ignored\n"
           "    mountpoint: directory over which to mount\n"
           "\n",
           progname);
}
