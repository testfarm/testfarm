##
## TestFarm
## TCP Networking helpers
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

LIBS = libtcp.a
BINS = tcpwrap tcpserver tcpclient

include ../../tools/defs.mk

ifdef GLIB
CPPFLAGS += -DENABLE_GLIB
endif

##### TARGETS
HEADERS = tcp.h
ifdef GLIB
HEADERS += tcpsrv.h
endif

##### SOURCES
LIBSRCS = tcpaddr.c tcpconnect.c
ifdef GLIB
LIBSRCS += tcpsrv.c
endif
ALLCSRCS = $(LIBSRCS) tcpwrap.c tcpserver.c tcpclient.c

##### $(BINS) and $(LIBS) DEPENDENCIES
CFLAGS += -I../useful
LDFLAGS += -L. -L../useful -ltcp -luseful

libtcp.a: $(LIBSRCS:%.c=%.o)

tcpwrap: tcpwrap.o
tcpserver: tcpserver.o
tcpclient: tcpclient.o

install:
	@echo >/dev/null

-include .deps
