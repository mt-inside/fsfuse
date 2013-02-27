#
# Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
#
# Unit tests makefile fragment.
#

HERE := $(ROOT)/tests/unit

vpath %.c $(HERE)

TEST_OBJS := hash_table_test.o      \
             http_test.o            \
             indexnode_test.o       \
             proto_indexnode_test.o \
             string_buffer_test.o   \
             uri_test.o             \
             utils_test.o

OBJECTS += $(TEST_OBJS)
