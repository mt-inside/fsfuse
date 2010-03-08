#
# Directory makefile fragment.
#
# Copyright (C) Matthew Turner 2008-2010. All rights reserved.
#
# $Id$
#

HERE := $(SRC_ROOT)

vpath %.c $(HERE)
vpath %.xml $(HERE)
vpath %.xsl $(HERE)

# Explicitly listed as not everything in this directory is built all the time
# fsfuse.o isn't listed because it isn't always wanted.
OUR_OBJS :=                         \
            alarms.o                \
            buildnumber.o           \
	    common.o                \
            config.o                \
            config_define.o         \
            direntry.o              \
            download_thread_pool.o  \
            fetcher.o               \
            hash.o                  \
            indexnode.o             \
	    inode_map.o             \
            localei.o               \
            locks.o                 \
            parser.o                \
            peerstats.o             \
            trace.o

OBJECTS += $(OUR_OBJS)
