##
## TestFarm
## Test Engine
##

GLIB = glib-2.0

CHECK_PACKAGES_deb += libpcre3-dev
CHECK_PACKAGES_rpm += pcre-devel

BINS = testfarm-engine

include ../../tools/defs.mk

##### OPTIONS
LIBDIRS = interface/useful interface/shell interface/tcp

CFLAGS += $(LIBDIRS:%=-I../../%)
LDFLAGS += $(LIBDIRS:%=-L../../%) -lshell -luseful -ltcp

CFLAGS  += $(shell pcre-config --cflags-posix)
LDFLAGS += $(shell pcre-config --libs-posix)

##### SOURCES
SOURCES = tengine.c periph_item.c periph.c trig.c listener.c expr.c result.c timer.c nibble.c \
	  run.c debug.c
OBJECTS = $(SOURCES:.c=.o)
ALLCSRCS = $(SOURCES)

##### $(BINS) and $(LIBS) DEPENDENCIES
testfarm-engine: $(OBJECTS)

install: install_bin

-include .deps
