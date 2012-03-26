#
# Directory makefile fragment.
#
# Copyright (C) Matthew Turner 2008-2012. All rights reserved.
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
            common.o                \
            config.o                \
            config_define.o         \
            direntry.o              \
            download_thread.o       \
            download_thread_pool.o  \
            fetcher.o               \
            hash_table.o            \
            http.o                  \
            indexnode.o             \
            indexnodes.o            \
            inode_map.o             \
            listing.o               \
            localei.o               \
            locks.o                 \
            parser.o                \
            peerstats.o             \
            string_buffer.o         \
            trace.o

OBJECTS += $(OUR_OBJS)
