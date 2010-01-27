#
# Directory makefile fragment.
#
# Copyright (C) Matthew Turner 2008-2010. All rights reserved.
#
# $Id: Makefile 464 2010-01-23 01:01:14Z matt $
#

HERE := $(SRC_ROOT)

vpath %.c $(HERE)
vpath %.xml $(HERE)
vpath %.xsl $(HERE)

# Explicitly listed as not everything in this directory is built all the time
# fsfuse.o isn't listed because it isn't always wanted.
OUR_OBJS := common.o                \
            buildnumber.o           \
            hash.o                  \
            trace.o                 \
            direntry.o              \
            fetcher.o               \
            parser.o                \
            download_thread_pool.o  \
            indexnode.o             \
            config.o                \
            config_define.o         \
            alarms.o                \
            locks.o                 \
            localei.o               \
            peerstats.o

OBJECTS += $(OUR_OBJS)
