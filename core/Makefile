SUBDIRS = config engine codegen output report icons gui logview perl wizards manualgui launch build

include ../tools/defs.mk

all:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d all; \
	done

clean::
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d clean; \
	done
