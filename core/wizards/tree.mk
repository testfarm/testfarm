##
## TestFarm
## Test Suite Development Wizards - Test Tree management Makefile
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

SCRIPT_MK = /opt/testfarm/lib/script.mk

ifneq ($(strip $(TREENAME)),)

.SUFFIXES: .tree
.IGNORE: clean

all: Makefile.d/wizdef.deps deps
	make -r -f $(SCRIPT_MK)

%.pm: Makefile.d/wizdef.deps deps
	make -r -f $(SCRIPT_MK) $@

clean:
	make -r -f $(SCRIPT_MK) clean

deps: Makefile.d/$(TREENAME).scripts.deps

Makefile.d/$(TREENAME).scripts.deps: Makefile.d/$(TREENAME).wiz.deps $(shell [ -f Makefile.d/$(TREENAME).wiz.deps ] && cat Makefile.d/$(TREENAME).wiz.deps)
	@echo '--- Remaking '$(TREENAME)' scripts dependencies...'
	twiz-deps -wiz @$< > $@
else
all:
	@echo '*** Environment variable TREENAME is required.'
	@false
clean:;
endif
