#
# Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
#
# Directory makefile fragment.
#

METHODS_HERE := $(SRC_ROOT)/fuse_methods

vpath %.c $(METHODS_HERE)

# Should list these explicitly, really
METHODS_SOURCES = $(wildcard $(METHODS_HERE)/*.c)
METHODS_OBJECTS = $(patsubst %.c,%.o,$(notdir $(METHODS_SOURCES)))

OBJECTS += $(METHODS_OBJECTS)
