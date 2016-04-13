##
##    TestFarm Virtual User -- Top-level build rules
##
##    This file is part of TestFarm,
##    the Test Automation Tool for Embedded Software.
##    Please visit http://www.testfarm.org.
##
##    TestFarm is free software: you can redistribute it and/or modify
##    it under the terms of the GNU General Public License as published by
##    the Free Software Foundation, either version 3 of the License, or
##    (at your option) any later version.
##
##    TestFarm is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##    GNU General Public License for more details.
##
##    You should have received a copy of the GNU General Public License
##    along with TestFarm.  If not, see <http://www.gnu.org/licenses/>.
##

PKGNAME := testfarm-vu

SUBDIRS = public common capture client display filters

include ../tools/defs.mk

all:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d all || exit 1; \
	done

install:
	$(RM) $(DESTDIR)
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d install || exit 1; \
	done

clean::
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d clean; \
	done
