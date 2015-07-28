PKGNAME = testfarm-core

SUBDIRS = config engine codegen output report icons gui logview perl wizards manualgui launch build

include ../tools/defs.mk

all:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d PKGNAME=$(PKGNAME) all || exit 1; \
	done

install:
	$(RM) $(DESTDIR)
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d PKGNAME=$(PKGNAME) install || exit 1; \
	done

clean::
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d PKGNAME=$(PKGNAME) clean; \
	done
