##
## TestFarm Virtual User -- Display tool
## Display Control Interface
##
## Author: Sylvain Giroudon
## Creation: 05-JAN-2004
##
## This file is part of TestFarm,
## the Test Automation Tool for Embedded Software.
## Please visit http://www.testfarm.org.
##
## TestFarm is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## TestFarm is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with TestFarm.  If not, see <http://www.gnu.org/licenses/>.
##

GTK = gtk+-2.0

CHECK_PACKAGES_deb += libpcre3-dev zlib1g-dev libpng12-dev libjpeg-dev
CHECK_PACKAGES_rpm += pcre-devel zlib-devel libpng-devel jpeg-devel

BINS = testfarm-vu-display

include ../../tools/defs.mk
include ../defs.mk

##### OPTIONS
LIBDIRS = interface/useful interface/link interface/shell vu/public vu/common

CFLAGS += $(LIBDIRS:%=-I../../%) -I./$(GTK)
LDFLAGS += $(LIBDIRS:%=-L../../%) -ltvu -ltvu-common -lz -lshell -luseful -ljpeg

CFLAGS  += $(shell pcre-config --cflags)
LDFLAGS += $(shell pcre-config --libs) -l pcreposix

CFLAGS  += $(shell pkg-config --cflags libpng)
LDFLAGS += $(shell pkg-config --libs libpng)

##### TARGETS
ICONS = testfarm-vu.png testfarm-vu-frame.png screenshot.png

DESKTOPDIR = $(INSTALLROOT)/usr/share/applications
ICONDIR = $(INSTALLDIR)/icons

##### SOURCES
GLADE_SRCS = $(GTK)/support.c $(GTK)/interface.c
DISPLAY_SRCS = main.c xpm.c style.c utils.c window_icon.c \
	       display.c display_desktop.c display_sel.c display_pad.c display_screenshot.c \
	       display_pattern.c display_refresh.c display_record.c display_cursor.c \
	       display_command.c viewer.c editor.c
DISPLAY_OBJS = $(GLADE_SRCS:%.c=%.o) $(DISPLAY_SRCS:%.c=%.o)

ALLCSRCS = $(DISPLAY_SRCS) $(GLADE_SRCS)

##### $(BINS) and $(LIBS) DEPENDENCIES
testfarm-vu-display: $(DISPLAY_OBJS)

install_desktop: testfarm-vu-display.desktop
	@$(MKDIR) $(DESKTOPDIR)
	@$(CP) -v $^ $(DESKTOPDIR)

install_icons: $(ICONS)
	$(MKDIR) $(ICONDIR)
	$(CP) -v $^ $(ICONDIR)

install: install_bin install_desktop install_icons

clean::
	$(RM) $(GTK)/*.o

-include .deps
