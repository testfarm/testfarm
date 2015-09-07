##
## TestFarm - Installation-related Resources
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

##### Platform config
GTK = gtk+-2.0
CHECK_PACKAGES_deb += libxml2-dev

LIBS = libinstall.a

include ../../tools/defs.mk

##### OPTIONS
LIBDIRS = interface/useful

CFLAGS += $(LIBDIRS:%=-I../../%)
LDFLAGS += $(LIBDIRS:%=-L../../%) -luseful

CFLAGS  += $(shell xml2-config --cflags)
LDFLAGS += $(shell xml2-config --libs)

##### TARGETS
HEADERS = install.h install_icon.h defaults.h

MIMEDIR = $(INSTALLROOT)/usr/share/mime/packages
PROFILEDIR = $(INSTALLROOT)/etc/profile.d
DESKTOPDIR = $(INSTALLROOT)/usr/share/applications

##### SOURCES
LIBSRCS = install.c install_font.c install_icon.c defaults.c
ALLCSRCS = $(LIBSRCS)

##### $(BINS) and $(LIBS) DEPENDENCIES
libinstall.a: $(LIBSRCS:%.c=%.o)

install_mime: testfarm.xml
	@$(MKDIR) $(MIMEDIR)
	@$(CP) -v $^ $(MIMEDIR)

install_desktop: testfarm.desktop
	@$(MKDIR) $(DESKTOPDIR)
	@$(CP) -v $^ $(DESKTOPDIR)

install_profile: testfarm.sh testfarm.csh
	@$(MKDIR) $(PROFILEDIR)
	@$(CP) -v $^ $(PROFILEDIR)

install: install_mime install_desktop install_profile

-include .deps
