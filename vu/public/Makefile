##
## TestFarm Virtual User -- Public ressources for external modules
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

LIBS = libtvu.a

include ../../tools/defs.mk
include ../defs.mk

##### TARGETS
HEADERS = frame_geometry.h frame_rgb.h frame_buf.h
BINS =
SCRIPTS =

##### SOURCES
LIB_SRCS = frame_geometry.c frame_rgb.c frame_buf.c
ALLCSRCS = $(LIB_SRCS)

##### $(BINS) and $(LIBS) DEPENDENCIES
libtvu.a: $(LIB_SRCS:%.c=%.o)

-include .deps
