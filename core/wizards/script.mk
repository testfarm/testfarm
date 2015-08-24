##
## TestFarm
## Test Suite Development Wizards - Test Script management Makefile
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

ifneq ($(strip $(TREENAME)),)

.SUFFIXES: .wizdef .wiz .pm
.IGNORE: clean

ifdef TESTFARM_WIZLIB
WIZLIB = $(subst :, ,$(TESTFARM_WIZLIB))
else
WIZLIB = wiz
endif

VPATH += $(WIZLIB)

WIZ_SCRIPTS = $(shell cat Makefile.d/$(TREENAME).wiz.deps 2>/dev/null)
PM_SCRIPTS = $(WIZ_SCRIPTS:%.wiz=%.pm)

ifeq ($(strip $(PM_SCRIPTS)),)
all:;
clean:;
else
all: $(PM_SCRIPTS)
clean:
	/bin/rm -f $(PM_SCRIPTS)
endif

%.pm: %.wiz
	twiz-script -q $<
	twiz-check $@

-include Makefile.d/wizdef.deps
-include Makefile.d/$(TREENAME).scripts.deps

else
all:
	@echo '*** Environment variable TREENAME is required.'
	@false
clean:;
endif
