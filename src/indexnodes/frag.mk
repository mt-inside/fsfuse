#
# Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
#
# Directory makefile fragment.
#

INDEXNODES_HERE := $(SRC_ROOT)/indexnodes

vpath %.c $(INDEXNODES_HERE)

# Should list these explicitly, really
INDEXNODES_SOURCES = $(wildcard $(INDEXNODES_HERE)/*.c)
INDEXNODES_OBJECTS = $(patsubst %.c,%.o,$(notdir $(INDEXNODES_SOURCES)))

OBJECTS += $(INDEXNODES_OBJECTS)
