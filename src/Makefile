#
# Makefile for fsfuse.
#
# Copyright (C) Matthew Turner 2008-2009. All rights reserved.
#
# $Id$
#

DIRENTRY_CACHE := 1
PROGRESS_METER := 0
DEBUG          := 1

#default
default: fsfuse

.PHONY: clean count

DEFINES := -DFUSE_USE_VERSION=26 -D_FILE_OFFSET_BITS=64 -D_REENTRANT

OBJECTS := fsfuse.o                \
           hash.o                  \
           trace.o                 \
           direntry.o              \
           fetcher.o               \
           parser.o                \
           download_thread_pool.o  \
           indexnode.o             \
           config.o                \
           alarms.o                \
           locks.o                 \
           fsfuse_ops/readdir.o    \
           fsfuse_ops/read.o       \
           fsfuse_ops/statfs.o     \
           fsfuse_ops/others.o

DEPENDS := $(patsubst %.o,%.d,$(OBJECTS))
#this will cause an attempt to build all included files (although it will still
#moan they don't exist first, just before building them, if given the chance)
-include $(DEPENDS)

ifeq ($(DIRENTRY_CACHE),1)
    OBJECTS += direntry_cache.o
    DEFINES += -DFEATURE_DIRENTRY_CACHE=1
endif
ifeq ($(PROGRESS_METER),1)
    OBJECTS += progress.o
    DEFINES += -DFEATURE_PROGRESS_METER=1
endif

INCLUDES = `curl-config --cflags` `xml2-config --cflags` -I.
OS:=$(shell uname -s)
ifeq ($(OS),Darwin)
    INCLUDES += -I/opt/local/include
endif

CFLAGS := -W -Wall -pedantic $(INCLUDES) $(DEFINES)
CFLAGS_OPTIM := -O2
CFLAGS_DEBUG := -O0 -g3
ifeq ($(DEBUG),1)
    CFLAGS += $(CFLAGS_DEBUG)
else
    CFLAGS += $(CFLAGS_OPTIM)
endif

LDFLAGS := -lfuse -lncurses `curl-config --libs` `xml2-config --libs`
ifeq ($(DEBUG),1)
    LDFLAGS += $(CFLAGS_DEBUG)
else
    LDFLAGS += $(CFLAGS_OPTIM)
endif
ifeq ($(OS),Darwin)
    LDFLAGS += -L/opt/local/lib
endif
ifeq ($(OS),FreeBSD)
    LDFLAGS += -L/usr/local/lib -liconv -pthread
endif


%.o: %.c
	@echo "Compiling $< -> $@"
	@$(CC) -c $(CFLAGS) -o $@ $<

%.d: %.c
	@echo "Dependencies $< -> $@"
	@$(CC) -MM $(CFLAGS) -o $@ $<

fsfuse: $(OBJECTS)
	@echo "Linking $@"
	@$(CC) $(LDFLAGS) -o $@ $(OBJECTS)

clean:
	rm -f fsfuse $(DEPENDS) $(OBJECTS) gmon.out fsfuse.log

count:
	find . \( -name "*c" -o -name "*h" \) -exec cat {} + | wc -l
