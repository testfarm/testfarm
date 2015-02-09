PKGNAME = testfarm-vu

SUBDIRS = public common capture client display ocr filters

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
