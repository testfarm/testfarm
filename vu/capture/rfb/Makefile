##
## TestFarm Virtual User -- RFB Device
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

SHARED = rfb.so

include ../../../tools/defs.mk
include ../../defs.mk

##### OPTIONS
VPATH += ..

LIBDIRS = interface/useful interface/tcp interface/link vu/public vu/common vu/capture

CFLAGS += $(LIBDIRS:%=-I../../../%)
LDFLAGS += $(LIBDIRS:%=-L../../../%)

CFLAGS += -fPIC
LDFLAGS += -rdynamic

##### SOURCES
MOD_SRCS = d3des.c rfblib.c rfbframe.c
RECORD_SRCS = rfbrecord.c
ALLCSRCS = $(MOD_SRCS) $(RECORD_SRCS)

##### $(BINS) and $(LIBS) DEPENDENCIES
all: $(SHARED)

rfb.so: $(MOD_SRCS:%.c=%.o)
vncrecord: $(RECORD_SRCS:%.c=%.o)

install_modules: $(SHARED)

install: install_bin install_modules

-include .deps
