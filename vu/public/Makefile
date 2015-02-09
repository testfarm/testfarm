##
## TestFarm Virtual User -- Public ressources for external modules
##

GLIB = glib-2.0

CHECK_PACKAGES_deb += libpcre3-dev
CHECK_PACKAGES_rpm += pcre-devel

LIBS = libtvu.a

include ../../tools/defs.mk
include ../defs.mk

##### TARGETS
HEADERS = frame_geometry.h frame_rgb.h frame_buf.h
BINS =
SCRIPTS =

##### SOURCES
LIB_SRCS = frame_geometry.c frame_rgb.c frame_buf.c
ALLCSRCS = $(LIB_SRCS)

##### $(BINS) and $(LIBS) DEPENDENCIES
libtvu.a: $(LIB_SRCS:%.c=%.o)

install: install_bin

-include .deps
