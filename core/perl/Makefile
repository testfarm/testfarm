##
## TestFarm - Perl Modules
##
## Author: Sylvain Giroudon
## Creation: 14-JUL-2006
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

CHECK_PACKAGES_deb += libsys-gamin-perl
CHECK_PACKAGES_rpm +=

include ../../tools/defs.mk

##### TARGETS
PERLLIBS = Version.pm Dialog.pm FAM.pm Locate.pm Config.pm Service.pm \
	   Engine.pm Trig.pm Exec.pm Env.pm \
	   Interface.pm ../../interface/link/InterfaceInput.pm \
	   Serial.pm
LIBDIR = $(INSTALLDIR)/lib/TestFarm

all: $(PERLLIBS)

clean::
	$(RM) Version.pm Dialog.pm

##### PERL libraries
Version.pm: Version.in.pm
	sed 's/@VERSION@/$(SRCVERSION)/g' $< >$@
Dialog.pm: Dialog.in.pm TDialog.glade
	./paste.pl __GLADE_INTERFACE__ $^ > $@

##### Installation
install_perllibs: $(PERLLIBS)
	@$(MKDIR) $(LIBDIR)
	@$(CP) -v $^ $(LIBDIR)

.PHONY: install
install: install_perllibs
