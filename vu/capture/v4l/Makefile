##
## TestFarm Virtual User -- V4L Capture Device
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

CHECK_PACKAGES_deb += libv4l-dev
CHECK_PACKAGES_rpm += 

SHARED = v4l.so v4l1.so

include ../../../tools/defs.mk
include ../../defs.mk

##### OPTIONS
VPATH += ..

LIBDIRS = interface/useful vu/public vu/common vu/capture

CFLAGS += $(LIBDIRS:%=-I../../../%)
LDFLAGS += $(LIBDIRS:%=-L../../../%)

CFLAGS += -fPIC
LDFLAGS += -rdynamic

##### SOURCES
DECODERS_SRCS = jpegdecode.c yuvdecode.c
V4L_SRCS = v4ldev.c
V4L1_SRCS = v4l1dev.c
ALLCSRCS = $(V4L_SRCS) $(V4L1_SRCS) $(DECODERS_SRCS)

DECODERS_OBJS = $(DECODERS_SRCS:%.c=%.o)
V4L_OBJS = $(V4L_SRCS:%.c=%.o) $(DECODERS_OBJS) $(CAPTURE_OBJS)
V4L1_OBJS = $(V4L1_SRCS:%.c=%.o) $(DECODERS_OBJS)  $(CAPTURE_OBJS)

##### $(BINS) and $(LIBS) DEPENDENCIES
all: $(SHARED)

v4l.so: $(V4L_OBJS)
v4l1.so: $(V4L1_OBJS)

install_modules: $(SHARED)

install: install_bin install_modules

-include .deps
