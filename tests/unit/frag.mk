#
# Unit tests makefile fragment.
#
# Copyright (C) Matthew Turner 2008-2010. All rights reserved.
#
# $Id: Makefile 464 2010-01-23 01:01:14Z matt $
#

HERE := $(ROOT)/tests/unit

vpath %.c $(HERE)

TEST_OBJS := hash_test.o   \
	     common_test.o

OBJECTS += $(TEST_OBJS)
