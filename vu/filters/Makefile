##
## TestFarm Virtual User -- Standard filters
##

GLIB = glib-2.0

BINS = pixel-join pixel-wipe pixel-dilate pixel-ds

include ../../tools/defs.mk
include ../defs.mk

##### OPTIONS
LIBDIRS = vu/public

CFLAGS += $(LIBDIRS:%=-I../../%)
LDFLAGS += $(LIBDIRS:%=-L../../%) -ltvu

##### TARGETS
FILTERLIBDIR = $(USRLIBDIR)/filters

##### SOURCES
JOIN_SRCS = pixel-join.c
WIPE_SRCS = pixel-wipe.c
DILATE_SRCS = pixel-dilate.c
DS_SRCS = pixel-ds.c
ALLCSRCS = main.c $(JOIN_SRCS) $(WIPE_SRCS) $(DILATE_SRCS) $(DS_SRCS)

##### $(BINS) and $(LIBS) DEPENDENCIES
pixel-join: main.o $(JOIN_SRCS:%.c=%.o)
pixel-wipe: main.o $(WIPE_SRCS:%.c=%.o)
pixel-dilate: main.o $(DILATE_SRCS:%.c=%.o)
pixel-ds: main.o $(DS_SRCS:%.c=%.o)

install_filters:
	$(MKDIR) $(FILTERLIBDIR)
	$(CP) $(BINS) $(FILTERLIBDIR)

install: install_filters

-include .deps
