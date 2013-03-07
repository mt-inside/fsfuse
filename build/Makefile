#
# Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
#
# Main makefile for fsfuse.
#

ROOT ?= ..
SRC_ROOT := $(ROOT)/src

# == Option Defaults ==

CC ?= gcc
#CC ?= clang

DIRENTRY_CACHE ?= 0
PROGRESS_METER ?= 0
DEBUG ?= 1

MAIN_OBJECT ?= fsfuse.o


# == Basics ==

default: fsfuse
.PHONY: default depend objects clean count build_revision.c

OS:=$(shell uname -s)


CONFIGS := config_declare.h \
           config_define.c

INCLUDES = -I. -I$(SRC_ROOT) -isystem $(SRC_ROOT)/3rdparty

CFLAGS = -std=c99 -Wall -Wextra -pedantic -Werror $(INCLUDES) -D_REENTRANT -DDEBUG=$(DEBUG) $(FEATURES)
CFLAGS += `curl-config --cflags` `xml2-config --cflags`
ifeq ($(OS),Darwin) # it would be nice to do this all the time, but fuse is usually installed from git on linux dev boxen.
    CFLAGS += `pkg-config --cflags fuse` -mmacosx-version-min=10.5
else
    CFLAGS += -D_FILE_OFFSET_BITS=64
endif
ifneq ($(OS),Darwin)
    # Linux needs both of these and acts sanely; BSD is needed for strdup(), POSIX
    #   for getaddrinfo(), etc.
    # MacOSX doesn't seem to need these, and in fact POSIX breaks the build on Mac
    CFLAGS += -D_POSIX_SOURCE -D_BSD_SOURCE
endif

CFLAGS_OPTIM := -O2
CFLAGS_DEBUG := -O0 -g3
ifeq ($(DEBUG),1)
    CFLAGS += $(CFLAGS_DEBUG)
else
    CFLAGS += $(CFLAGS_OPTIM)
endif

LDFLAGS = `curl-config --libs` `xml2-config --libs`
ifeq ($(OS),Darwin)
    LDFLAGS += `pkg-config --libs fuse`
else
    LDFLAGS += -lfuse
endif
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
    FEATURES += -DFEATURE_DIRENTRY_CACHE=1
endif
ifeq ($(PROGRESS_METER),1)
    OBJECTS += progress.o
    FEATURES += -DFEATURE_PROGRESS_METER=1
    CFLAGS += `ncurses5-config --cflags`
    LDFLAGS += `ncurses5-config --libs`
endif


# == Sources ==

include $(SRC_ROOT)/frag.mk
include $(SRC_ROOT)/fuse_methods/frag.mk
include $(SRC_ROOT)/indexnodes/frag.mk


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

build_revision.c:
	@echo "Making build_revision.c"
	$(shell /bin/echo -n 'const char * const build_revision = "'  > build_revision.c)
	$(shell /bin/echo -n `git show --pretty=format:'%h' HEAD | head -n1` >> build_revision.c)
	$(shell /bin/echo    '";'                             >> build_revision.c)

%.i: %.c
	@echo "Preprocessing $< -> $@"
	@$(CC) -E $(CFLAGS) -o $@ $<

%.o: %.c
	@echo "Compiling $< -> $@"
	@$(CC) -c $(CFLAGS) -o $@ $<

%.s: %.c
	@echo "Assembling $< -> $@"
	@$(CC) -S $(CFLAGS) -o $@ $<

%.d: %.c
	@echo "Dependencies $< -> $@"
	@$(CC) -MM -MG $(CFLAGS) -o $@ $<

fsfuse: $(CONFIGS) objects build_revision.o
	@echo "Linking $@"
	@$(CC) -o $@ $(OBJECTS) build_revision.o $(LDFLAGS)

depend: $(DEPENDS)

objects: $(OBJECTS)

clean:
	rm -f fsfuse $(DEPENDS) $(OBJECTS) $(CONFIGS) build_revision.c build_revision.o gmon.out

count:
	find . \( -name "$(SRC_ROOT)/*.c" -o -name "$(SRC_ROOT)/*.h" \) -exec cat {} + | wc -l
