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


# == Basics ==

default: fsfuse
.PHONY: objects clean count

OS:=$(shell uname -s)

DEFINES = -DFUSE_USE_VERSION=26 -D_FILE_OFFSET_BITS=64 -D_REENTRANT

# This is necessary for fuse.h to be valid. We also use a lot of other features
# that also require this macro.
DEFINES += -D_BSD_SOURCE

OBJECTS := fsfuse.o                \
           common.o                \
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

OPS_FILES = $(wildcard fsfuse_ops/*.c)
OPS_OBJECTS = $(patsubst %.c,%.o,$(OPS_FILES))
OBJECTS += $(OPS_OBJECTS)

CONFIGS := config_declare.h \
           config_define.c

#This variable must be set unconditionally, otherwise the .d files will never
#be cleaned
DEPENDS = $(patsubst %.o,%.d,$(OBJECTS))
ifeq "$(findstring clean,$(MAKECMDGOALS))" ""
#this will cause an attempt to build all included files (although it will still
#moan they don't exist first, just before building them, if given the chance)
-include $(DEPENDS)
endif

INCLUDES = `curl-config --cflags` `xml2-config --cflags` -I. -isystem 3rdparty

CFLAGS = -std=c99 -Wall -Wextra -pedantic $(INCLUDES) -DDEBUG=$(DEBUG) $(DEFINES)
CFLAGS_OPTIM := -O2
CFLAGS_DEBUG := -O0 -g3
ifeq ($(OS),Darwin)
    CFLAGS += -mmacosx-version-min=10.5
endif
ifeq ($(DEBUG),1)
    CFLAGS += $(CFLAGS_DEBUG)
else
    CFLAGS += $(CFLAGS_OPTIM)
endif

LDFLAGS = -lfuse `curl-config --libs` `xml2-config --libs`
ifeq ($(DEBUG),1)
    LDFLAGS += $(CFLAGS_DEBUG)
else
    LDFLAGS += $(CFLAGS_OPTIM)
endif
ifeq ($(OS),FreeBSD)
    LDFLAGS += -L/usr/local/lib -liconv -pthread
endif


# == Optional Features ==

ifeq ($(DIRENTRY_CACHE),1)
    OBJECTS += direntry_cache.o
    DEFINES += -DFEATURE_DIRENTRY_CACHE=1
endif
ifeq ($(PROGRESS_METER),1)
    OBJECTS += progress.o
    DEFINES += -DFEATURE_PROGRESS_METER=1
    LDFLAGS += -lncurses
endif


# == Build Rules ==

config_declare.h: config-config.xml config.xsl
	xsltproc --stringparam filetype h config.xsl config-config.xml > config_declare.h
config_define.c: config-config.xml config.xsl
	xsltproc --stringparam filetype c config.xsl config-config.xml > config_define.c

buildnumber.c:
	$(shell echo "unsigned svn_rev = " > buildnumber.c)
	$(shell echo `svn info | grep Revision | cut -d ' ' -f 2` >> buildnumber.c)
	$(shell echo ";" >> buildnumber.c)

%.o: %.c
	@echo "Compiling $< -> $@"
	@$(CC) -c $(CFLAGS) -o $@ $<

%.d: %.c
	@echo "Dependencies $< -> $@"
	@$(CC) -MM -MG $(CFLAGS) -o $@ $<

fsfuse: $(CONFIGS) objects
	@$(MAKE) buildnumber.o
	@echo "Linking $@"
	@$(CC) $(LDFLAGS) -o $@ $(OBJECTS)

objects: ${OBJECTS}

clean:
	rm -f fsfuse $(DEPENDS) $(OBJECTS) $(CONFIGS) buildnumber.c gmon.out fsfuse.log

count:
	find . \( -name "*c" -o -name "*h" \) -exec cat {} + | wc -l
