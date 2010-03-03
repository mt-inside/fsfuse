/*
 * Common header file, including important constants and over-rides. To be
 * included by every source file.
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
#define FSFUSE_VERSION   "0.4.0 beta"
#define FSFUSE_DATE      "February 2010"
#define FSFUSE_COPYRIGHT "Copyright (C) Matthew Turner 2008-2010\n"

/* Greatest protocol version that we know we support */
#define PROTO_MINIMUM "0.5"
#define PROTO_MAXIMUM "0.13"

#define NOT_USED(x) (void)x
#define DO_NOTHING NOT_USED(0)
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#define MIN(x,y) (((x) < (y)) ? (x) : (y))

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

#define FSFUSE_BLKSIZE   512


/* General utility functions */
extern int common_init (void);
extern void common_finalise (void);

extern char *fsfuse_dirname (const char *path);
extern char *fsfuse_basename (const char *path);

extern unsigned fsfuse_get_thread_index (void);

extern int compare_dotted_version (const char *ver, const char *cmp);

extern int is_ip4_address (const char *s);
extern int is_ip6_address (const char *s);

#endif /* _included_common_h */
