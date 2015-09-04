##
## TestFarm
## Miscellaneous Useful Stuffs
##
## Author: Sylvain Giroudon
## Creation: 17-AUG-1999
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

LIBS = libuseful.a

BINS = child_test
ifdef GLIB
BINS += filelist_test
endif

include ../../tools/defs.mk

ifdef GLIB
CFLAGS += -DENABLE_GLIB
endif

##### TARGETS
HEADERS = useful.h pty.h child.h sig.h
ifdef GLIB
HEADERS += filelist.h
endif

##### SOURCES
LIBSRCS = strread.c strcleanup.c strskip.c strupper.c strtimestamp.c strxml.c \
	  sleep_ms.c sig.c \
	  pty.c fgets2.c \
	  child.c child_test.c
ifdef GLIB
LIBSRCS += filelist.c
endif
ALLCSRCS = $(LIBSRCS)

##### RULES
libuseful.a: $(LIBSRCS:%.c=%.o)

child_test: child_test.o child.o
filelist_test: filelist_test.o filelist.o

install:
	@echo >/dev/null

-include .deps
