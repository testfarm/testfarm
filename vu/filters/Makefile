##
## TestFarm Virtual User -- Standard filters
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

BINS = pixel-join pixel-wipe pixel-dilate pixel-ds

include ../../tools/defs.mk
include ../defs.mk

##### OPTIONS
LIBDIRS = vu/public

CFLAGS += $(LIBDIRS:%=-I../../%)
LDFLAGS += $(LIBDIRS:%=-L../../%) -ltvu

##### TARGETS
FILTERLIBDIR = $(USRLIBDIR)/filters

##### SOURCES
JOIN_SRCS = pixel-join.c
WIPE_SRCS = pixel-wipe.c
DILATE_SRCS = pixel-dilate.c
DS_SRCS = pixel-ds.c
ALLCSRCS = main.c $(JOIN_SRCS) $(WIPE_SRCS) $(DILATE_SRCS) $(DS_SRCS)

##### $(BINS) and $(LIBS) DEPENDENCIES
pixel-join: main.o $(JOIN_SRCS:%.c=%.o)
pixel-wipe: main.o $(WIPE_SRCS:%.c=%.o)
pixel-dilate: main.o $(DILATE_SRCS:%.c=%.o)
pixel-ds: main.o $(DS_SRCS:%.c=%.o)

install_filters:
	$(MKDIR) $(FILTERLIBDIR)
	$(CP) $(BINS) $(FILTERLIBDIR)

install: install_filters

-include .deps
