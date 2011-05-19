#
# Unit tests makefile fragment.
#
# Copyright (C) Matthew Turner 2008-2011. All rights reserved.
#
# $Id$
#

HERE := $(ROOT)/tests/unit

vpath %.c $(HERE)

TEST_OBJS := common_test.o        \
             hash_test.o          \
             http_test.o          \
             string_buffer_test.o

OBJECTS += $(TEST_OBJS)
