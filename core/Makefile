PKGNAME = testfarm-core

SUBDIRS = config engine codegen output report icons gui logview perl wizards manualgui launch build

include ../tools/defs.mk

all:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d PKGNAME=$(PKGNAME) all; \
	done

install:
	$(RM) $(DESTDIR)
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d PKGNAME=$(PKGNAME) install; \
	done

clean::
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d PKGNAME=$(PKGNAME) clean; \
	done
