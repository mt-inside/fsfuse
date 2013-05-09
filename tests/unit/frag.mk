#
# Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
#
# Unit tests makefile fragment.
#

TEST_HERE := $(ROOT)/tests/unit

vpath %.c $(TEST_HERE)

TEST_OBJS :=                        \
             config_test.o          \
             indexnode_test.o       \
             indexnodes_list_test.o \
             parser_xml_test.o      \
             parser_test.o          \
             parser_stubs.o         \
             proto_indexnode_test.o \
             ref_count_test.o       \
             string_buffer_test.o   \
             utils_test.o

TEST_OBJS += indexnode_stubs.o

TEST_OBJS += testdata_path.o
testdata_path.c:
	@echo "Making $@"
	$(shell /bin/echo -n 'const char * const testdata_path = "'  > $@)
	$(shell /bin/echo -n $(TEST_HERE)"/testdata"                >> $@)
	$(shell /bin/echo    '";'                                   >> $@)

OBJECTS += $(TEST_OBJS)
