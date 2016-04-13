##
## TestFarm
## Global build rule
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

SUBDIRS = interface core vu ocr

all:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d all || exit 1; \
	done

deb:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d deb || exit 1; \
	done

rpm:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d rpm || exit 1; \
	done

clean::
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d clean; \
	done
