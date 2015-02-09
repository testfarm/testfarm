#
# Commands
#
MKDIR = mkdir -p
RM = /bin/rm -rf
CP = /bin/cp -f
MV = /bin/mv -f

CPP = $(CROSS_COMPILE)gcc -E
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
RANLIB = $(CROSS_COMPILE)ranlib

#
# Directories
#
ARCH ?= $(shell arch)

ROOT_DIR = $(shell git rev-parse --show-cdup 2>/dev/null)
ROOT_DIR ?= .
TOOLS_DIR = $(ROOT_DIR)/tools
STAGING_DIR = $(ROOT_DIR)/tmp

PKGDIR = $(STAGING_DIR)/$(PKGNAME)/$(ARCH)
DESTDIR = $(PKGDIR)/root
INSTALLROOT = $(DESTDIR)
INSTALLDIR = $(DESTDIR)/opt/testfarm

#
# Version numbers
#
GITBRANCH = $(shell git rev-parse --abbrev-ref HEAD 2>/dev/null)
ifeq ($(GITBRANCH),)
  APPVERSION = 0.0
  SRCVERSION = 0.0.0
else
  APPVERSION = $(shell $(TOOLS_DIR)/gitversion.sh --short)
  SRCVERSION = $(shell $(TOOLS_DIR)/gitversion.sh)
endif

BUILDDATE = $(shell date +%y%m%d)

VERSION = $(SRCVERSION)-$(BUILDDATE)

#
# Package check
#
ifdef GLIB
CHECK_PACKAGES_deb += libglib2.0-dev
CHECK_PACKAGES_rpm += glib2-devel
endif

ifdef GTK
CHECK_PACKAGES_deb += libgtk2.0-dev
CHECK_PACKAGES_rpm += gtk2-devel
endif

#
# Compile flags
#
CFLAGS = -Wall -DVERSION=\"$(SRCVERSION)\"
LDFLAGS =

ifdef GLIB
CFLAGS  += $(shell pkg-config --cflags $(GLIB)) -DENABLE_GLIB
LDFLAGS += $(shell pkg-config --libs $(GLIB))
endif

ifdef GTK
CFLAGS  += $(shell pkg-config --cflags $(GTK))
LDFLAGS += $(shell pkg-config --libs $(GTK))
endif

#
# Compile rules
#
.PHONY: check libs bins install

all: check libs bins

libs:: $(LIBS)

bins:: $(BINS)

%.o: %.c
	$(CC) -o $@ $< -c $(CFLAGS)

ifdef LIBS
$(LIBS):
	$(AR) rv $@ $^
	$(RANLIB) $@
endif

ifdef SHARED
$(SHARED):
	$(LD) -shared -nostartfiles -o $@ $^
endif

ifdef BINS
$(BINS):
	$(LD) -o $@ $^ $(LDFLAGS)
endif

clean::
	$(RM) $(BINS) $(LIBS) $(SHARED) *.o *~

install: $(INSTALLDIR)

install_shared: $(SHARED)
	$(MKDIR) $(INSTALLDIR)/lib
	$(CP) $^ $(INSTALLDIR)/lib/

install_bin: $(BINS)
	$(MKDIR) $(INSTALLDIR)/bin
	$(CP) $^ $(INSTALLDIR)/bin/

$(INSTALLDIR):
	$(MKDIR) $(INSTALLDIR)

mrproper:: clean
	$(RM) $(STAGING_DIR)

include $(TOOLS_DIR)/check.mk
include $(TOOLS_DIR)/pkg.mk
