##
## TestFarm
## Graphical User Interface
##
## Author: Sylvain Giroudon
## Creation: 12-APR-2000
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

GTK = gtk+-2.0

CHECK_PACKAGES_deb += libxml2-dev libssl-dev
CHECK_PACKAGES_rpm += libxml2-devel openssl-devel

BINS = testfarm-run

include ../../tools/defs.mk

##### OPTIONS
LIBDIRS = interface/shell interface/useful interface/tcp core/codegen core/output core/report core/config

CFLAGS += $(LIBDIRS:%=-I../../%) -I./$(GTK)
LDFLAGS += $(LIBDIRS:%=-L../../%) -loutput -lcodegen -lshell -luseful -ltcp -lreport -linstall -lm

CFLAGS  += $(shell xml2-config --cflags)
LDFLAGS += $(shell xml2-config --libs)

CFLAGS  += $(shell pkg-config --cflags libcrypto)
LDFLAGS += $(shell pkg-config --libs libcrypto)

##### SOURCES
GUI_SRCS = main.c pipe.c options.c properties.c color.c style.c tree_run.c \
	   perl_run.c tree_gui.c file_gui.c status_gui.c xpm_gui.c report_gui.c report_progress.c \
	   output_gui.c validate_gui.c system_gui.c error_gui.c check_gui.c input_gui.c
GLADE_SRCS = $(GTK)/support.c $(GTK)/interface.c
ALLCSRCS = $(GUI_SRCS) $(GLADE_SRCS:%=$(GTK)/%)

##### $(BINS) and $(LIBS) DEPENDENCIES
testfarm-run: $(GLADE_SRCS:%.c=%.o) $(GUI_SRCS:%.c=%.o)

.PHONY: install
install: install_bin

clean::
	$(RM) $(GTK)/*.o

-include .deps
