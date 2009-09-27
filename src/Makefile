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

#Force rebuilding every build
.PHONY: buildnumber.c

DEFINES := -DFUSE_USE_VERSION=26 -D_FILE_OFFSET_BITS=64 -D_REENTRANT

OBJECTS := fsfuse.o                \
           buildnumber.o           \
           hash.o                  \
           trace.o                 \
           direntry.o              \
           fetcher.o               \
           parser.o                \
           download_thread_pool.o  \
           indexnode.o             \
           config.o                \
           config_define.o 	   \
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

CONFIGS := config_declare.h \
	   config_define.c

ifeq ($(DIRENTRY_CACHE),1)
    OBJECTS += direntry_cache.o
    DEFINES += -DFEATURE_DIRENTRY_CACHE=1
endif
ifeq ($(PROGRESS_METER),1)
    OBJECTS += progress.o
    DEFINES += -DFEATURE_PROGRESS_METER=1
endif

INCLUDES = `curl-config --cflags` `xml2-config --cflags` -I. -isystem 3rdparty
OS:=$(shell uname -s)
ifeq ($(OS),Darwin)
    INCLUDES += -I/opt/local/include
endif

CFLAGS := -W -Wall -pedantic $(INCLUDES) $(DEFINES)
CFLAGS_OPTIM := -O2
CFLAGS_DEBUG := -O0 -g3 -DDEBUG
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
	@$(CC) -MM $(CFLAGS) -o $@ $<

fsfuse: $(CONFIGS) $(OBJECTS)
	@echo "Linking $@"
	@$(CC) $(LDFLAGS) -o $@ $(OBJECTS)

clean:
	rm -f fsfuse $(DEPENDS) $(OBJECTS) $(CONFIGS) buildnumber.c gmon.out fsfuse.log

count:
	find . \( -name "*c" -o -name "*h" \) -exec cat {} + | wc -l
