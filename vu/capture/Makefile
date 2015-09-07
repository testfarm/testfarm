##
## TestFarm Virtual User -- Capture Devices
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

include ../../tools/defs.mk
include ../defs.mk

SUBDIRS = rfb v4l file

##### OPTIONS
LIBDIRS = interface/useful vu/public vu/common

CFLAGS += $(LIBDIRS:%=-I../../%)
LDFLAGS += $(LIBDIRS:%=-L../../%)

CFLAGS += -fPIC

##### SOURCES
LIB_SRCS = fps.c capture_device.c capture_frame.c
PROC_SRCS = capture_proc_agent.c capture_proc_interface.c
ALLCSRCS = $(LIB_SRCS) $(PROC_SRCS)

LIB_OBJS = $(LIB_SRCS:%.c=%.o)
PROC_OBJS = $(PROC_SRCS:%.c=%.o)

##### RULES
all: $(LIB_OBJS) $(PROC_OBJS)
	@for i in $(SUBDIRS);\
	  do (cd $$i; $(MAKE) all);\
	done

install:
	@for i in $(SUBDIRS);\
	  do (cd $$i; $(MAKE) install);\
	done

deps::
	@for i in $(SUBDIRS);\
	  do (cd $$i; $(MAKE) deps);\
	done

clean::
	@for i in $(SUBDIRS);\
	  do (cd $$i; $(MAKE) clean);\
	done

-include .deps
