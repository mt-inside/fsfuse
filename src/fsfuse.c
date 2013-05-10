/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * This file contains the programme's entry point, command line parsing, event
 * loop, etc.
 */

#include "common.h"

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fuse/fuse_lowlevel.h>
#include <fuse/fuse_opt.h>

#include <sys/utsname.h>
#include <curl/curl.h>
#include <libxml/xmlversion.h>

#include "buildnumber.h"
#include "config_manager.h"
#include "config_reader.h"
#include "direntry.h"
#include "download_thread_pool.h"
#include "fetcher.h"
#include "indexnodes.h"
#include "localei.h"
#include "peerstats.h"
#include "string_buffer.h"
#include "utils.h"

#include "fuse_methods.h"


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


static int settings_tryget_config_file            (int argc, char *argv[], const char **config_file_path);
static start_action_t settings_parse_command_line (int argc, char *argv[]);
static int my_fuse_main (void);
static void fuse_args_set (struct fuse_args *fuse_args, config_reader_t *config);
static void fsfuse_splash (void);
static void fsfuse_versions (void);
static void fsfuse_usage (void);


int main(int argc, char *argv[])
{
    int rc = EXIT_FAILURE;
    char **myargv = malloc(argc * sizeof(char *));
    const char *config_file;
    start_action_t sa;


    progname = argv[0];

    fsfuse_splash();

    /* Check system fuse version */
    if (fuse_version() < FUSE_USE_VERSION)
    {
        trace_warn("%s error: fuse version %d required, only version %d available\n",
                   progname, FUSE_USE_VERSION, fuse_version());
        goto pre_init_bail;
    }


    /* These stack; later files over-ride earlier ones. */
    config_manager_add_from_file( strdup( "/etc/fsfuserc" ) );
    config_manager_add_from_file( strdup( "~/.fsfuserc" ) );
    config_manager_add_from_file( strdup( "./fsfuserc" ) );

    /* Perfunctory parse of the command line just to find the name of the config
     * file, if specified. This has to be done first, so we can then load
     * settings from it, /before/ parsing the command line proper, over-writing
     * any config settings with higher-priority command line values */
    memcpy(myargv, argv, argc * sizeof(char *));
    if (settings_tryget_config_file(argc, myargv, &config_file))
    {
        config_manager_add_from_file( strdup( config_file ) );
    }
    free(myargv);


    /* Parse cmdline args */
    /* fuse *really* wants you to use its parsing mechanism. It ignores
     * fuse_args.argv[0], as if it's the program name - i.e. it's expecting a
     * copy of your argv, not a special one you've made for it. However, if you
     * give fuse_new() any arguments it doesn't understand it b0rks. This is
     * convenient, given fuse's parsing functions allow you to remove any
     * (non-fuse) arguments you've dealt with...
     * The argv member of the fuse_args structure you give it must be malloc()d,
     * as it all gets realloc()d and free()d a lot. Giving it NULL works too.
     * In response, we copy argv[0] over to fuse's array (in case it does
     * anything with it) and fill the rest with the args we want it to see */
    sa = settings_parse_command_line(argc, argv);

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
    if (trace_init()                ||
        utils_init()                ||
        locale_init()               ||
        fetcher_init()              ||
        direntry_init()             ||
        download_thread_pool_init()    )
    {
        trace_error("initialisation failed\n");
        goto pre_init_bail;
    }


    rc = my_fuse_main();


    /* finalisations */
    download_thread_pool_finalise();
    direntry_finalise();
    fetcher_finalise();
    locale_finalise();
    utils_finalise();
    trace_finalise();

pre_init_bail:
    config_singleton_delete( );
    if (mountpoint.real) free(mountpoint.real);
    if (mountpoint.error) free(mountpoint.error);


    exit(rc);
}

static int my_fuse_main (void)
{
    int rc = EXIT_FAILURE;
    struct fuse_chan *ch;
    struct fuse_session *se;
    struct fuse_args fuse_args;
    fsfuse_ctxt_t fsfuse_ctxt;
    config_reader_t *config = config_get_reader();


    fuse_args_set(&fuse_args, config);


    /* Hand over to fuse */
    if ((ch = fuse_mount(mountpoint.real, &fuse_args)))
    {
        if ((se = fuse_lowlevel_new(&fuse_args, &fuse_methods, sizeof(fuse_methods), &fsfuse_ctxt)))
        {
            /* Setup */
            fuse_set_signal_handlers(se);
            fuse_session_add_chan(se, ch);
            fuse_daemonize(config_proc_fg(config));

            /* Go! */
            if (config_proc_singlethread(config))
            {
                rc = fuse_session_loop(se);
            }
            else
            {
                rc = fuse_session_loop_mt(se);
            }

            /* Teardown */
            fuse_session_remove_chan(ch);
            fuse_remove_signal_handlers(se);
            fuse_session_destroy(se);
            fuse_unmount(mountpoint.real, ch);
        }
    }

    fuse_opt_free_args(&fuse_args);


    return rc;
}

/* Performs a minimal parse of the command line args to get the config file
 * location */
static int settings_tryget_config_file (int argc, char *argv[], const char **config_file_path)
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
                *config_file_path = strdup(optarg);
                return 1;
                break;

            case '?':
                break;

            default:
                assert(0);
        }
    }

    return 0;
}

static start_action_t settings_parse_command_line (int argc, char *argv[])
{
    start_action_t rc = start_action_MOUNT;
    int proc_debug,        proc_debug_set = 0,
        proc_fg,           proc_fg_set = 0,
        proc_singlethread, proc_singlethread_set = 0;
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"config",         required_argument, NULL, 'c'},
        {"debug",          no_argument,       NULL, 'd'},
        {"foreground",     no_argument,       NULL, 'f'},
        {"singlethreaded", no_argument,       NULL, 's'},
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
            case 'c': /* config file - dealt with in settings_get_config_file() */
                break;

            case 'd': /* debug */
                proc_debug = 1;
                proc_debug_set = 1;
                /* fall through; d => f */

            case 'f': /* foreground */
                proc_fg = 1;
                proc_fg_set = 1;
                break;

            case 's': /* single threaded */
                proc_singlethread = 1;
                proc_singlethread_set = 1;
                break;

            case 't': /* trace */
#if DEBUG
#define TRACE_ARG(aREA)                          \
                if (!strcmp(optarg, #aREA))      \
                {                                \
                    aREA##_trace_on();           \
                }
#else
#define TRACE_ARG(aREA)
#endif

                TRACE_ARG(direntry)
                TRACE_ARG(dtp)
                TRACE_ARG(fetcher)
                TRACE_ARG(method)
                TRACE_ARG(peerstats)
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

            case 'o': /* generated by mount - ignore for now */
                break;

            default:
                assert(0);
        }
    }

    config_manager_add_from_cmdline(
        proc_debug_set,        proc_debug,
        proc_fg_set,           proc_fg,
        proc_singlethread_set, proc_singlethread
    );

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

static void fuse_args_set (struct fuse_args *fuse_args, config_reader_t *config)
{
    string_buffer_t *my_arg = string_buffer_new();


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
    if (config_proc_debug(config))
    {
        string_buffer_printf(my_arg, "-d");
        fuse_opt_add_arg(fuse_args, string_buffer_peek(my_arg));
    }

    string_buffer_printf(my_arg, "-ofsname=" FSFUSE_NAME);
    fuse_opt_add_arg(fuse_args, string_buffer_peek(my_arg));

    string_buffer_printf(my_arg, "-osubtype=" FSFUSE_NAME "-" FSFUSE_VERSION);
    fuse_opt_add_arg(fuse_args, string_buffer_peek(my_arg));

    string_buffer_printf(my_arg, "-oallow_other");
    fuse_opt_add_arg(fuse_args, string_buffer_peek(my_arg));

    string_buffer_printf(my_arg, "-oro");
    fuse_opt_add_arg(fuse_args, string_buffer_peek(my_arg));

    string_buffer_delete(my_arg);
}

static void fsfuse_splash (void)
{
    printf("%s - FragSoc Filesystem in USErspace\n"
           "Version %s - %s (commit %s)\n"
           "%s\n\n",
           FSFUSE_NAME,
           FSFUSE_VERSION,
           FSFUSE_DATE,
           build_revision,
           FSFUSE_COPYRIGHT
          );
}

static void fsfuse_versions (void)
{
    {
        struct utsname un;

        uname(&un);

        printf("Using kernel %s %s\n"
               "Compiled with %s %s\n"
               "Using libfuse version %d (built against %d, using API version %d)\n"
               "Using %s\n"
               "Using libxml2 %s",
               un.sysname, un.release,
#if defined(__clang__)
               "clang",
               __clang_version__,
#elif defined(__llvm__)
               "llvm-gcc",
               __VERSION__,
#elif defined(__GNUC__)
               "gcc",
               __VERSION__,
#else
               "Unknown compiler",
               "Unknown version",
#endif
               fuse_version(), FUSE_VERSION, FUSE_USE_VERSION,
               curl_version(),
               LIBXML_DOTTED_VERSION /* This is the version of the headers on this machine, but xmlParserVersion is ugly */
              );

        printf("\n");
    }

    {
        struct fuse_args fuse_args;
        string_buffer_t *my_arg = string_buffer_new();
        fuse_args.argv = NULL;
        fuse_args.argc = 0;

        string_buffer_printf(my_arg, "-d");
        fuse_opt_add_arg(&fuse_args, string_buffer_peek(my_arg));

        string_buffer_printf(my_arg, "--version");
        fuse_opt_add_arg(&fuse_args, string_buffer_peek(my_arg));

        fuse_parse_cmdline(&fuse_args, NULL, NULL, NULL);

        string_buffer_delete(my_arg);

        printf("\n");
    }
}

static void fsfuse_usage (void)
{
    printf("usage: %s device mountpoint [-o option[,...]]\n"
           "    device: device to mount - ignored\n"
           "    mountpoint: directory over which to mount\n"
           "\n",
           progname);
}
