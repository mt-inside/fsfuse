#
# Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
#
# Directory makefile fragment.
#

PARSER_HERE := $(SRC_ROOT)/parser

vpath %.c $(PARSER_HERE)

# Should list these explicitly, really
PARSER_SOURCES = $(wildcard $(PARSER_HERE)/*.c)
PARSER_OBJECTS = $(patsubst %.c,%.o,$(notdir $(PARSER_SOURCES)))

OBJECTS += $(PARSER_OBJECTS)
