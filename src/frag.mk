#
# Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
#
# Directory makefile fragment.
#

HERE := $(SRC_ROOT)

vpath %.c $(HERE)
vpath %.xml $(HERE)
vpath %.xsl $(HERE)

# Explicitly listed as not everything in this directory is built all the time
# fsfuse.o isn't listed because it isn't always wanted.
OUR_OBJS :=                         \
            alarms.o                \
            config.o                \
            config_define.o         \
            curl_utils.o            \
            direntry.o              \
            download_thread.o       \
            download_thread_pool.o  \
            fetcher.o               \
            hash_table.o            \
            http.o                  \
            indexnode.o             \
            indexnodes.o            \
            indexnodes_iterator.o   \
            indexnodes_list.o       \
            inode_map.o             \
            listing.o               \
            localei.o               \
            locks.o                 \
            parser.o                \
            peerstats.o             \
            proto_indexnode.o       \
            ref_count.o             \
            string_buffer.o         \
            trace.o                 \
            uri.o                   \
            utils.o

OBJECTS += $(OUR_OBJS)
