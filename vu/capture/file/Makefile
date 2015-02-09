##
## TestFarm Virtual User -- File Capture Device
##

GLIB = glib-2.0

CHECK_PACKAGES_deb += libpcre3-dev zlib1g-dev libjpeg-dev
CHECK_PACKAGES_rpm += pcre-devel zlib-devel jpeg-devel

SHARED = file.so
BINS = testfarm-vu-file

include ../../../tools/defs.mk
include ../../defs.mk

##### OPTIONS
VPATH += ..

LIBDIRS = interface/useful vu/public vu/common vu/capture

CFLAGS += $(LIBDIRS:%=-I../../../%)
LDFLAGS += $(LIBDIRS:%=-L../../../%)

CFLAGS += -fPIC
LDFLAGS += -rdynamic
LDFLAGS += -ltvu-common -ltvu
LDFLAGS += -luseful -ljpeg -lz #-lefence

#CFLAGS  += $(shell pkg-config --cflags $(GTHREAD))
#LDFLAGS += $(shell pkg-config --libs $(GTHREAD))

CFLAGS  += $(shell pcre-config --cflags)
LDFLAGS += $(shell pcre-config --libs) -l pcreposix

CFLAGS  += $(shell pkg-config --cflags libpng)
LDFLAGS += $(shell pkg-config --libs libpng)

##### SOURCES
FILE_SRCS = filedev.c
ALLCSRCS = $(FILE_SRCS)

CAPTURE_OBJS = capture_device.o capture_frame.o fps.o
FILE_OBJS = $(FILE_SRCS:%.c=%.o) $(CAPTURE_OBJS)

##### $(BINS) and $(LIBS) DEPENDENCIES
all: $(SHARED)

testfarm-vu-file: $(FILE_OBJS) proc.o capture_proc_agent.o
file.so: proc.o capture_proc_interface.o

install_modules: $(SHARED)

install: install_bin install_modules

-include .deps
