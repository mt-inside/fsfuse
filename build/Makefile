#
# Main makefile for fsfuse.
#
# Copyright (C) Matthew Turner 2008-2010. All rights reserved.
#
# $Id$
#

ROOT ?= ..
SRC_ROOT := $(ROOT)/src

# == Option Defaults ==

DIRENTRY_CACHE ?= 0
PROGRESS_METER ?= 0
DEBUG ?= 1

MAIN_OBJECT ?= fsfuse.o


# == Basics ==

default: fsfuse
.PHONY: default depend objects clean count buildnumber.c

OBJECTS+=buildnumber.o

OS:=$(shell uname -s)

DEFINES = -D_FILE_OFFSET_BITS=64 -D_REENTRANT

# This is necessary for fuse.h to be valid. We also use a lot of other features
# that also require this macro.
DEFINES += -D_BSD_SOURCE
#getaddrinfo() et all
DEFINES += -D_POSIX_SOURCE

CONFIGS := config_declare.h \
           config_define.c

INCLUDES = `curl-config --cflags` `xml2-config --cflags` -I. -I$(SRC_ROOT) -isystem $(SRC_ROOT)/3rdparty

CFLAGS = -std=c99 -Wall -Wextra -pedantic -Werror $(INCLUDES) -DDEBUG=$(DEBUG) $(DEFINES)
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

OBJECTS += $(MAIN_OBJECT)

ifeq ($(DIRENTRY_CACHE),1)
    OBJECTS += direntry_cache.o
    DEFINES += -DFEATURE_DIRENTRY_CACHE=1
endif
ifeq ($(PROGRESS_METER),1)
    OBJECTS += progress.o
    DEFINES += -DFEATURE_PROGRESS_METER=1
    LDFLAGS += -lncurses
endif


# == Sources ==

include $(SRC_ROOT)/frag.mk
include $(SRC_ROOT)/fsfuse_ops/frag.mk


# == Dependencies ==

#Must be included after the lists of objects are included!

#This variable must be set unconditionally, otherwise the .d files will never
#be cleaned
DEPENDS = $(patsubst %.o,%.d,$(OBJECTS))
ifeq "$(findstring clean,$(MAKECMDGOALS))" ""
#this will cause an attempt to build all included files (although it will still
#moan they don't exist first, just before building them, if given the chance)
-include $(DEPENDS)
endif


# == Build Rules ==

config_declare.h: config.xsl config-config.xml
	xsltproc --stringparam filetype h $^ > $@
config_define.c:  config.xsl config-config.xml
	xsltproc --stringparam filetype c $^ > $@

buildnumber.c:
	@echo "Making buildnumber.c"
	$(shell echo "unsigned svn_rev = " > buildnumber.c)
	$(shell echo `svn info $(SRC_ROOT) | grep Revision | cut -d ' ' -f 2` >> buildnumber.c)
	$(shell echo ";" >> buildnumber.c)

%.o: %.c
	@echo "Compiling $< -> $@"
	@$(CC) -c $(CFLAGS) -o $@ $<

%.s: %.c
	@echo "Assembling $< -> $@"
	@$(CC) -S $(CFLAGS) -o $@ $<

%.d: %.c
	@echo "Dependencies $< -> $@"
	@$(CC) -MM -MG $(CFLAGS) -o $@ $<

fsfuse: $(CONFIGS) objects
	@echo "Linking $@"
	@$(CC) $(LDFLAGS) -o $@ $(OBJECTS)

depend: $(DEPENDS)

objects: $(OBJECTS)

clean:
	rm -f fsfuse $(DEPENDS) $(OBJECTS) $(CONFIGS) buildnumber.c gmon.out

count:
	find . \( -name "$(SRC_ROOT)/*.c" -o -name "$(SRC_ROOT)/*.h" \) -exec cat {} + | wc -l
