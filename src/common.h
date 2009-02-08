/*
 * Common header file, including important constants and over-rides. To be
 * included by every source file.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#ifndef _included_common_h
#define _included_common_h

#include <stdio.h>
#include <signal.h>
#include <assert.h>

#include "trace.h"


#define FSFUSE_NAME    "fsfuse"
#define FSFUSE_VERSION "0.1.1"
#define FSFUSE_DATE    "December 2008"

/* Greatest protocol version that we know we support */
#define PROTO_GREATEST 0.5f

#define NOT_USED(x) (void)x
#define DO_NOTHING NOT_USED(0)
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#define MIN(x,y) (((x) < (y)) ? (x) : (y))


#define FSFUSE_BLKSIZE   512


#endif /* _included_common_h */
