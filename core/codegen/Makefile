##
## TestFarm
## Test Suite code generator
##
## Author: Sylvain Giroudon
## Creation: 31-MAY-2000
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

CHECK_PACKAGES_deb += libxml2-dev
CHECK_PACKAGES_rpm += libxml2-devel

LIBS = libcodegen.a
BINS = codegen

include ../../tools/defs.mk

##### OPTIONS
LIBDIRS = interface/useful core/config

CFLAGS += $(LIBDIRS:%=-I../../%)
LDFLAGS += $(LIBDIRS:%=-L../../%) -luseful -linstall

CFLAGS  += $(shell xml2-config --cflags)
LDFLAGS += $(shell xml2-config --libs)

##### TARGETS
HEADERS = codegen_tree.h codegen_tree_error.h codegen_tree_check.h codegen_tree_show.h \
	  codegen_script.h codegen_code.h codegen_criticity.h codegen.h

##### SOURCES
LIBSRCS = codegen_tree.c codegen_tree_error.c codegen_tree_init.c codegen_tree_precond.c \
	  codegen_tree_scan.c codegen_tree_check.c codegen_tree_show.c \
	  codegen_script.c codegen_code.c codegen_criticity.c
ALLCSRCS = $(LIBSRCS) codegen.c

##### $(BINS) and $(LIBS) DEPENDENCIES
libcodegen.a: $(LIBSRCS:.c=.o)
codegen: codegen.o libcodegen.a

.PHONY: install
install:
	@echo >/dev/null

-include .deps
