##
## TestFarm
## Log Viewer
##
## Author: Sylvain Giroudon
## Creation: 06-MAY-2004
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
