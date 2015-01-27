##
## TestFarm
## Data Logger Interface
##
## Author: Sylvain Giroudon
## Creation: 24-MAY-2000
##

GLIB = glib-2.0

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
