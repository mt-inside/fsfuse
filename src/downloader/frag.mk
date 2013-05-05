#
# Copyright (C) 2008-2012 Matthew Turner. Distributed under the GPL v3.
#
# Directory makefile fragment.
#

DOWNLOADER_HERE := $(SRC_ROOT)/downloader

vpath %.c $(DOWNLOADER_HERE)

# Should list these explicitly, really
DOWNLOADER_SOURCES = $(wildcard $(DOWNLOADER_HERE)/*.c)
DOWNLOADER_OBJECTS = $(patsubst %.c,%.o,$(notdir $(DOWNLOADER_SOURCES)))

OBJECTS += $(DOWNLOADER_OBJECTS)
