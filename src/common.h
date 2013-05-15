/*
 * Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
 *
 * Common header file, including important constants and over-rides. To be
 * included FIRST by every source file.
 */

#ifndef _INCLUDED_COMMON_H
#define _INCLUDED_COMMON_H

#include <assert.h>

#include "trace.h"


#define typeof __typeof__

#define FSFUSE_NAME      "fsfuse"
#define FSFUSE_VERSION   "0.5.0"
#define FSFUSE_DATE      "August 2012"
#define FSFUSE_COPYRIGHT "Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3."

/* Use fuse API version provided on this (the build) system up to the maximum
 * version that we support */
#define MAX_FUSE_VERSION 30
#define FUSE_USE_VERSION MIN(FUSE_VERSION, MAX_FUSE_VERSION)

/* Greatest protocol version that we know we support */
#define PROTO_MINIMUM "0.5"
#define PROTO_MAXIMUM "0.13"

#define NOT_USED(x) (void)(x)
#define DO_NOTHING NOT_USED(0)
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#define SWAP(x,y) { typeof(y) tMP = (y); (y) = (x); (x) = (tMP); }

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

#endif /* _INCLUDED_COMMON_H */
