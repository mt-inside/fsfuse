/*
 * Common header file, including important constants and over-rides. To be
 * included FIRST by every source file.
 *
 * Copyright (C) Matthew Turner 2008-2010. All rights reserved.
 *
 * $Id$
 */

#ifndef _included_common_h
#define _included_common_h

#include <stdio.h>
#include <signal.h>
#include <assert.h>

#include "trace.h"


#define FSFUSE_NAME      "fsfuse"
#define FSFUSE_VERSION   "0.5.0 beta"
#define FSFUSE_DATE      "February 2011"
#define FSFUSE_COPYRIGHT "Copyright (C) Matthew Turner 2008-2012"

/* Use fuse API version provided on this (the build) system up to the maximum
 * version that we support */
#define MAX_FUSE_VERSION 29
#define FUSE_USE_VERSION MIN(FUSE_VERSION, MAX_FUSE_VERSION)

/* Greatest protocol version that we know we support */
#define PROTO_MINIMUM "0.5"
#define PROTO_MAXIMUM "0.13"

#define NOT_USED(x) (void)x
#define DO_NOTHING NOT_USED(0)
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#define MIN(x,y) (((x) < (y)) ? (x) : (y))

#define free_const(x) free((void *)(x))

#if DEBUG
#define CALLER_DECL const char *const file, size_t line_num,
#define CALLER_DECL_ONLY const char *const file, size_t line_num
#define CALLER_INFO __FILE__, __LINE__,
#define CALLER_INFO_ONLY __FILE__, __LINE__
#define CALLER_PASS file, line_num,
#define CALLER_PASS_ONLY file, line_num
#define CALLER_FORMAT "%s:%zu"
#else
#define CALLER_DECL
#define CALLER_DECL_ONLY void
#define CALLER_INFO
#define CALLER_INFO_ONLY
#define CALLER_PASS
#define CALLER_PASS_ONLY
#define CALLER_FORMAT
#endif

#define FSFUSE_ROOT_INODE 1
#define FSFUSE_BLKSIZE   512


typedef struct _uri_t uri_t;


/* General utility functions */
extern int common_init (void);
extern void common_finalise (void);

extern char *fsfuse_dirname (const char *path);
extern char *fsfuse_basename (const char *path);

extern unsigned fsfuse_get_thread_index (void);

extern int compare_dotted_version (const char *ver, const char *cmp);

extern int is_ip4_address (const char *s);
extern int is_ip6_address (const char *s);

/* See RFC2396.
 * We assume a "generic uri" with a "server-based naming authority".
 * e.g.:
 *
 * scheme://userinfo@host:port/path?query#fragment
 *          \-- authority  --/
 */
extern uri_t *uri_new (void);
extern uri_t *uri_from_string (const char *str);
extern void uri_delete (uri_t *uri);

extern void uri_set_scheme   (uri_t *uri, const char *scheme  );
extern void uri_set_userinfo (uri_t *uri, const char *userinfo);
extern void uri_set_host     (uri_t *uri, const char *host    );
extern void uri_set_port     (uri_t *uri, const char *port    );
extern void uri_set_path     (uri_t *uri, const char *path    );
extern void uri_set_query    (uri_t *uri, const char *query   );
extern void uri_set_fragment (uri_t *uri, const char *fragment);

extern char *uri_get           (uri_t *uri);
extern char *uri_get_scheme    (uri_t *uri);
extern char *uri_get_authority (uri_t *uri);
extern char *uri_get_userinfo  (uri_t *uri);
extern char *uri_get_host      (uri_t *uri);
extern char *uri_get_port      (uri_t *uri);
extern char *uri_get_path      (uri_t *uri);
extern char *uri_get_query     (uri_t *uri);
extern char *uri_get_fragment  (uri_t *uri);

extern int hash_equal (char *h1, char *h2);

#endif /* _included_common_h */
