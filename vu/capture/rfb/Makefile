##
## TestFarm Virtual User -- RFB Device
##

GLIB = glib-2.0

SHARED = rfb.so

include ../../../tools/defs.mk
include ../../defs.mk

##### OPTIONS
VPATH += ..

LIBDIRS = interface/useful interface/tcp interface/link vu/public vu/common vu/capture

CFLAGS += $(LIBDIRS:%=-I../../../%)
LDFLAGS += $(LIBDIRS:%=-L../../../%)

CFLAGS += -fPIC
LDFLAGS += -rdynamic

##### SOURCES
MOD_SRCS = d3des.c rfblib.c rfbframe.c
RECORD_SRCS = rfbrecord.c
ALLCSRCS = $(MOD_SRCS) $(RECORD_SRCS)

##### $(BINS) and $(LIBS) DEPENDENCIES
all: $(SHARED)

rfb.so: $(MOD_SRCS:%.c=%.o)
vncrecord: $(RECORD_SRCS:%.c=%.o)

install_modules: $(SHARED)

install: install_bin install_modules

-include .deps
