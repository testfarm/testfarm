##
## TestFarm
## Data Logger Interface
##
## Author: Sylvain Giroudon
## Creation: 24-MAY-2000
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

CHECK_PACKAGES_deb += libglib2.0-dev
CHECK_PACKAGES_rpm += 

BINS =
LIBS = liblink.a

include ../../tools/defs.mk

ifdef GLIB
CFLAGS += -DENABLE_GLIB
endif

##### TARGETS
HEADERS = link_cmd.h tstamp.h link.h link_bin.h tcpip.h sub.h log.h
ifdef GLIB
HEADERS += link_txt.h
endif

##### SOURCES
LIB_SRCS = link_cmd.c link.c link_bin.c tcpip.c sub.c log.c
ifdef GLIB
LIB_SRCS += link_txt.c
endif

ALLCSRCS = $(LIB_SRCS)

##### $(BINS) and $(LIBS) DEPENDENCIES
CFLAGS += -I../useful -I../shell -I../tcp

liblink.a: $(LIB_SRCS:%.c=%.o)

install:
	@echo >/dev/null

-include .deps
