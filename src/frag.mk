#
# Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
#
# Directory makefile fragment.
#

SRC_HERE := $(SRC_ROOT)

vpath %.c $(SRC_HERE)
vpath %.xml $(SRC_HERE)
vpath %.xsl $(SRC_HERE)

# Explicitly listed as not everything in this directory is built all the time
# fsfuse.o isn't listed because it isn't always wanted.
SRC_OBJECTS :=                         \
               alarm_simple.o          \
               config_loader.o         \
               config_manager.o        \
               config_reader.o         \
               direntry.o              \
               download_thread.o       \
               download_thread_pool.o  \
               fetcher.o               \
               fs2_constants.o         \
               inode_map.o             \
               listing.o               \
               localei.o               \
               locks.o                 \
               parser.o                \
               peerstats.o             \
               ref_count.o             \
               string_buffer.o         \
               trace.o                 \
               utils.o

OBJECTS += $(SRC_OBJECTS)
