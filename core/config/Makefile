##
## TestFarm
## Installation-related Resources
##

##### Platform config
GTK = gtk+-2.0

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

.PHONY: install
install: install_mime install_desktop install_profile

-include .deps
