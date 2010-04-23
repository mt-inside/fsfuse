#
# Unit tests makefile fragment.
#
# Copyright (C) Matthew Turner 2008-2010. All rights reserved.
#
# $Id$
#

HERE := $(ROOT)/tests/unit

vpath %.c $(HERE)

TEST_OBJS := hash_test.o   \
             http_test.o   \
             common_test.o

OBJECTS += $(TEST_OBJS)
