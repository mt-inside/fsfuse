#
# Integration tests makefile fragment.
#
# Copyright (C) Matthew Turner 2008-2012. All rights reserved.
#
# $Id: frag.mk 595 2012-03-27 13:53:09Z matt $
#

HERE := $(ROOT)/tests/integration

vpath %.c $(HERE)

TEST_OBJS := list_test.o \
             read_test.o

OBJECTS += $(TEST_OBJS)
