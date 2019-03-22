#
# Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
#
# Main makefile for fsfuse.
#

SRC_ROOT := $(ROOT)/src

# == Option Defaults ==

CC := gcc
#CC := clang

DEBUG ?= 1

MAIN_OBJECT ?= fsfuse.o


# == Basics ==

.DEFAULT_GOAL := fsfuse
#doesn't seem to do anything
.DEFAULT: fsfuse
.PHONY: depend objects clean count build_revision.c

OS:=$(shell uname -s)


INCLUDES = -I. -I$(SRC_ROOT) -isystem $(SRC_ROOT)/3rdparty

CFLAGS = -std=c99 -Wall -Wextra -pedantic -Werror $(INCLUDES) -D_REENTRANT -DDEBUG=$(DEBUG) $(FEATURES)
CFLAGS += `curl-config --cflags` `xml2-config --cflags`
CFLAGS_OPTIM := -O2
CFLAGS_DEBUG := -O0 -g3
ifeq ($(OS),Darwin) # it would be nice to do this all the time, but fuse is usually installed from git on linux dev boxen.
    CFLAGS += `pkg-config --cflags fuse` -mmacosx-version-min=10.5
else
    CFLAGS += -D_FILE_OFFSET_BITS=64
endif
# _POSIX_SOURCE breaks the build on Darwin
# _POSIX_SOURCE also breaks the build on recent (9+ ?) FreeBSD (causes missing types in pthread.h)
ifeq ($(OS),FreeBSD)
    CFLAGS += -D_BSD_SOURCE
endif
ifeq ($(OS),Linux)
    # Seems to be equivalent to _POSIX_SOURCE and _BSD_SOURCE on newer systems
    # Both of these are needed; BSD is needed for strdup(), POSIX for
    #   getaddrinfo(), etc.
    CFLAGS += -D_DEFAULT_SOURCE
endif
ifeq ($(DEBUG),1)
    CFLAGS += $(CFLAGS_DEBUG)
else
    CFLAGS += $(CFLAGS_OPTIM)
endif

LDFLAGS = `curl-config --libs` `xml2-config --libs` -lcheck -pthread
ifeq ($(OS),Darwin)
    LDFLAGS += `pkg-config --libs fuse`
else
    LDFLAGS += -lfuse
endif
ifeq ($(OS),FreeBSD)
    LDFLAGS += -L/usr/local/lib -liconv
endif
ifneq ($(OS),Darwin)
    LDFLAGS += -pthread
endif
ifeq ($(DEBUG),1)
    LDFLAGS += $(CFLAGS_DEBUG)
else
    LDFLAGS += $(CFLAGS_OPTIM)
endif


# == Optional Features ==

OBJECTS += $(MAIN_OBJECT)


# == Sources ==

include $(SRC_ROOT)/frag.mk
include $(SRC_ROOT)/config/frag.mk
include $(SRC_ROOT)/downloader/frag.mk
include $(SRC_ROOT)/fuse_methods/frag.mk
include $(SRC_ROOT)/indexnodes/frag.mk
include $(SRC_ROOT)/nativefs/frag.mk
include $(SRC_ROOT)/parser/frag.mk


# == Dependencies ==

#Must be included after the lists of objects are included!

#This variable must be set unconditionally, otherwise the .d files will never
#be cleaned
DEPENDS = $(OBJECTS:.o=.d)
ifeq "$(findstring clean,$(MAKECMDGOALS))" ""
#this will cause an attempt to build all included files (although it will still
#moan they don't exist first, just before building them, if given the chance)
-include $(DEPENDS)
endif


# == Build Rules ==

build_revision.c:
	@echo "Making $@"
	$(shell /bin/echo -n 'const char * const build_revision = "'          > $@)
	$(shell /bin/echo -n `git show --pretty=format:'%h' HEAD | head -n1` >> $@)
	$(shell /bin/echo    '";'                                            >> $@)

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

fsfuse: $(AUTO_SOURCES) objects build_revision.o
	@echo "Linking $@"
	@$(CC) -o $@ $(OBJECTS) build_revision.o $(LDFLAGS)

depend: $(DEPENDS)

objects: $(OBJECTS)

clean:
	rm -f fsfuse $(DEPENDS) $(OBJECTS) $(AUTO_SOURCES) build_revision.c build_revision.o testdata_path.c testdata_path.o gmon.out

count:
	find $(SRC_ROOT) \( -name "*.c" -o -name "*.h" \) -exec cat {} + | wc -l
