##
## TestFarm Virtual User -- Command Interface
##

GLIB = glib-2.0

CHECK_PACKAGES_deb += libxml2-dev libpcre3-dev zlib1g-dev libpng12-dev
CHECK_PACKAGES_rpm += libxml2-devel pcre-devel zlib-devel libpng-devel

BINS = testfarm-vu testfarm-vu-grab

include ../../tools/defs.mk
include ../defs.mk

##### OPTIONS
LIBDIRS = interface/useful interface/tcp interface/link interface/shell vu/public vu/common vu/capture

CFLAGS += $(LIBDIRS:%=-I../../%)
LDFLAGS += $(LIBDIRS:%=-L../../%) -ltvu -ltvu-common -llink -lshell -lz -ltcp -luseful -ljpeg #-lefence

CFLAGS  += $(shell pkg-config --cflags gmodule-2.0)
LDFLAGS += $(shell pkg-config --libs gmodule-2.0)

#CFLAGS  += $(shell pkg-config --cflags gthread-2.0)
#LDFLAGS += $(shell pkg-config --libs gthread-2.0)

CFLAGS  += $(shell xml2-config --cflags)
LDFLAGS += $(shell xml2-config --libs)

CFLAGS  += $(shell pcre-config --cflags)
LDFLAGS += $(shell pcre-config --libs) -l pcreposix

CFLAGS  += $(shell pkg-config --cflags libpng)
LDFLAGS += $(shell pkg-config --libs libpng)

##### SOURCES
CLIENT_SRCS = main.c options.c tstamp.c error.c capture.c \
	      frame_command.c frame.c frame_display.c km.c grab.c agent.c \
	      match_command.c match.c match_link.c \
	      match_image.c match_image_map.c \
	      match_text.c ocr.c \
	      pad.c pad_list.c \
	      filter.c filter_thresh.c filter_ext.c
GRAB_SRCS = testfarm-vu-grab.c
ALLCSRCS = $(CLIENT_SRCS) $(GRAB_SRCS)

##### $(BINS) and $(LIBS) DEPENDENCIES
testfarm-vu: $(CLIENT_SRCS:%.c=%.o)
testfarm-vu-grab: $(GRAB_SRCS:%.c=%.o)

install_perl: VU.pm
	@$(MKDIR) $(PERLLIBDIR)
	$(CP) $^ $(PERLLIBDIR)

install: install_bin install_perl

-include .deps
