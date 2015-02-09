##
## TestFarm Virtual User -- Common ressources
##

GLIB = glib-2.0

CHECK_PACKAGES_deb += libpcre3-dev zlib1g-dev libjpeg-dev libpng12-dev
CHECK_PACKAGES_rpm += pcre-devel zlib-devel jpeg-devel libpng-devel

LIBS = libtvu-common.a

include ../../tools/defs.mk
include ../defs.mk

##### OPTIONS
LIBDIRS = interface/useful interface/link vu/public

CFLAGS += $(LIBDIRS:%=-I../../%)
LDFLAGS += $(LIBDIRS:%=-L../../%)

CFLAGS  += $(shell pcre-config --cflags)
LDFLAGS += $(shell pcre-config --libs)

CFLAGS  += $(shell pkg-config --cflags libpng)
LDFLAGS += $(shell pkg-config --libs libpng)

##### SOURCES
LIB_SRCS = user_dir.c frame_ctl.c frame_ctl_cmd.c frame_ctl_rsp.c \
	   keysyms.c keysyms.dat.c scroll.c \
	   color.c fuzz.c ppm.c png_file.c jpeg.c image_file.c record.c \
	   pattern.c pattern_mask.c pattern_image.c pattern_text.c
ALLCSRCS = $(LIB_SRCS)

##### $(BINS) and $(LIBS) DEPENDENCIES
keysyms.dat: /usr/include/X11/keysymdef.h
	(egrep '^#define[[:space:]]+XK_|^#ifdef[[:space:]]+XK_' $^ \
	| sed 's/^#define[[:space:]]XK_//g' \
	| sed 's/^#ifdef[[:space:]]XK_//g' \
	| awk '{printf "%s %s\n",$$1,$$2}'> $@)

keysyms.dat.c: keysyms.dat keysyms.h
	(echo "#include <stdio.h>" > $@)
	(echo "#include <X11/keysym.h>" >> $@)
	(echo "#include \"keysyms.h\"" >> $@)
	(echo "keysyms_t keysyms[] = {" >> $@)
	(awk '{ printf "  {\"%s\", %s},\n",$$1,$$2}' $< >> $@)
	(echo "  {NULL, 0}," >> $@)
	(echo "};" >> $@)

libtvu-common.a: $(LIB_SRCS:%.c=%.o)

install: install_bin

-include .deps
