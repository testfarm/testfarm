##
## TestFarm
## TCP Networking helpers
##
## Author: Sylvain Giroudon
## Creation: 17-AUG-1999
##

GLIB = glib-2.0

LIBS = libtcp.a
BINS = tcpwrap tcpserver tcpclient

include ../../tools/defs.mk

ifdef GLIB
CPPFLAGS += -DENABLE_GLIB
CFLAGS  += $(shell pkg-config --cflags $(GLIB))
LDFLAGS += $(shell pkg-config --libs $(GLIB))
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
