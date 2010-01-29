#
# Directory makefile fragment.
#
# Copyright (C) Matthew Turner 2008-2010. All rights reserved.
#
# $Id$
#

HERE := $(SRC_ROOT)/fsfuse_ops

vpath %.c $(HERE)

# Should list these explicitly, really
OPS_SOURCES = $(wildcard $(HERE)/*.c)
OPS_OBJECTS = $(patsubst %.c,%.o,$(notdir $(OPS_SOURCES)))

OBJECTS += $(OPS_OBJECTS)
