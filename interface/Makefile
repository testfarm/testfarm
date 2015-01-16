SUBDIRS = useful tcp shell link

include ../tools/defs.mk

all:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d all; \
	done

clean::
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d clean; \
	done

