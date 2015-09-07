##
## TestFarm
## Test Engine
##
## This file is part of TestFarm,
## the Test Automation Tool for Embedded Software.
## Please visit http://www.testfarm.org.
##
## TestFarm is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## TestFarm is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with TestFarm.  If not, see <http://www.gnu.org/licenses/>.
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
