##
## TestFarm
## Log Viewer
##
## Author: Sylvain Giroudon
## Creation: 06-MAY-2004
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

CHECK_PACKAGES_deb += libgamin-dev
CHECK_PACKAGES_rpm += gamin-devel

BINS = testfarm-logview

include ../../tools/defs.mk

##### OPTIONS
LIBDIRS = interface/useful core/config

CFLAGS += $(LIBDIRS:%=-I../../%)
LDFLAGS += $(LIBDIRS:%=-L../../%) -luseful -linstall #-lefence

GTK = gtk+-2.0
CFLAGS  += $(shell pkg-config --cflags $(GTK))
LDFLAGS += $(shell pkg-config --libs $(GTK))

CFLAGS  += $(shell pkg-config --cflags gamin)
LDFLAGS += $(shell pkg-config --libs gamin)

##### SOURCES
LOGVIEW_SRCS = logview.c filter.c list.c viewer.c
ALLCSRCS = $(LOGVIEW_SRCS)

##### $(BINS) and $(LIBS) DEPENDENCIES
testfarm-logview: $(LOGVIEW_SRCS:%.c=%.o)

.PHONY: install
install: install_bin

-include .deps
