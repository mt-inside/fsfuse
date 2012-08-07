#
# Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
#
# Integration tests makefile fragment.
#

HERE := $(ROOT)/tests/integration

vpath %.c $(HERE)

TEST_OBJS := list_test.o \
             read_test.o

OBJECTS += $(TEST_OBJS)
