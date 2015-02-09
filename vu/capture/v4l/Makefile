##
## TestFarm Virtual User -- V4L Capture Device
##

GLIB = glib-2.0

CHECK_PACKAGES_deb += libv4l-dev
CHECK_PACKAGES_rpm += 

SHARED = v4l.so v4l1.so

include ../../../tools/defs.mk
include ../../defs.mk

##### OPTIONS
VPATH += ..

LIBDIRS = interface/useful vu/public vu/common vu/capture

CFLAGS += $(LIBDIRS:%=-I../../../%)
LDFLAGS += $(LIBDIRS:%=-L../../../%)

CFLAGS += -fPIC
LDFLAGS += -rdynamic

##### SOURCES
DECODERS_SRCS = jpegdecode.c yuvdecode.c
V4L_SRCS = v4ldev.c
V4L1_SRCS = v4l1dev.c
ALLCSRCS = $(V4L_SRCS) $(V4L1_SRCS) $(DECODERS_SRCS)

DECODERS_OBJS = $(DECODERS_SRCS:%.c=%.o)
V4L_OBJS = $(V4L_SRCS:%.c=%.o) $(DECODERS_OBJS) $(CAPTURE_OBJS)
V4L1_OBJS = $(V4L1_SRCS:%.c=%.o) $(DECODERS_OBJS)  $(CAPTURE_OBJS)

##### $(BINS) and $(LIBS) DEPENDENCIES
all: $(SHARED)

v4l.so: $(V4L_OBJS)
v4l1.so: $(V4L1_OBJS)

install_modules: $(SHARED)

install: install_bin install_modules

-include .deps
