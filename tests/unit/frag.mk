#
# Unit tests makefile fragment.
#
# Copyright (C) Matthew Turner 2008-2012. All rights reserved.
#
# $Id$
#

HERE := $(ROOT)/tests/unit

vpath %.c $(HERE)

TEST_OBJS := hash_table_test.o    \
             http_test.o          \
             string_buffer_test.o \
             uri_test.o           \
             utils_test.o

OBJECTS += $(TEST_OBJS)
