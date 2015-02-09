##
## TestFarm Virtual User -- Capture Devices
##

GLIB = glib-2.0

include ../../tools/defs.mk
include ../defs.mk

SUBDIRS = rfb v4l file

##### OPTIONS
LIBDIRS = interface/useful vu/public vu/common

CFLAGS += $(LIBDIRS:%=-I../../%)
LDFLAGS += $(LIBDIRS:%=-L../../%)

CFLAGS += -fPIC

##### SOURCES
LIB_SRCS = fps.c capture_device.c capture_frame.c
PROC_SRCS = capture_proc_agent.c capture_proc_interface.c
ALLCSRCS = $(LIB_SRCS) $(PROC_SRCS)

LIB_OBJS = $(LIB_SRCS:%.c=%.o)
PROC_OBJS = $(PROC_SRCS:%.c=%.o)

##### RULES
all: $(LIB_OBJS) $(PROC_OBJS)
	@for i in $(SUBDIRS);\
	  do (cd $$i; $(MAKE) all);\
	done

install:
	@for i in $(SUBDIRS);\
	  do (cd $$i; $(MAKE) install);\
	done

deps::
	@for i in $(SUBDIRS);\
	  do (cd $$i; $(MAKE) deps);\
	done

clean::
	@for i in $(SUBDIRS);\
	  do (cd $$i; $(MAKE) clean);\
	done

-include .deps
