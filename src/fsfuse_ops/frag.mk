#
# Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
#
# Directory makefile fragment.
#

HERE := $(SRC_ROOT)/fsfuse_ops

vpath %.c $(HERE)

# Should list these explicitly, really
OPS_SOURCES = $(wildcard $(HERE)/*.c)
OPS_OBJECTS = $(patsubst %.c,%.o,$(notdir $(OPS_SOURCES)))

OBJECTS += $(OPS_OBJECTS)
