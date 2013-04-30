#
# Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
#
# Nativefs makefile fragment.
#

NATIVEFS_HERE := $(SRC_ROOT)/nativefs

vpath %.c $(NATIVEFS_HERE)

# Should list these explicitly, really
NATIVEFS_SOURCES = $(wildcard $(NATIVEFS_HERE)/*.c)
NATIVEFS_OBJECTS = $(patsubst %.c,%.o,$(notdir $(NATIVEFS_SOURCES)))

OBJECTS += $(NATIVEFS_OBJECTS)
