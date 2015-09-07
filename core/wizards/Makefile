##
## TestFarm - Test Suite Development Wizards
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

include ../../tools/defs.mk

##### TARGETS
PERLLIBDIR = $(INSTALLDIR)/lib/TestFarm
PERLLIBS = Wiz.pm
PERLBINS = twiz-conf twiz-script twiz-check twiz-makefile twiz-deps twiz-pod
MANUALS = twiz-script.1

all: $(MANUALS)

clean::
	$(RM) $(MANUALS)

##### Manuals
twiz-script.1: twiz-script.pl

%.pl %.1:
	pod2man $^ > $@

##### Installation
install_perlbins:
	@$(MKDIR) $(INSTALLDIR)/bin
	for bin in $(PERLBINS); do \
	  $(CP) -v $$bin.pl $(INSTALLDIR)/bin/$$bin ;\
	done

install_perlibs: $(PERLLIBS)
	@$(MKDIR) $(PERLLIBDIR)
	@$(CP) -v $^ $(PERLLIBDIR)

install_man: $(MANUALS)
	@$(MKDIR) $(INSTALLDIR)/man/man1
	@$(CP) -v $^ $(INSTALLDIR)/man/man1

install_xml: config.dtd config.xsl
	@$(MKDIR) $(INSTALLDIR)/lib
	@$(CP) -v $^ $(INSTALLDIR)/lib

install_makefile: build.mk tree.mk script.mk
	@$(MKDIR) $(INSTALLDIR)/lib
	@$(CP) -v $^ $(INSTALLDIR)/lib

install: install_perlibs install_perlbins install_xml install_man install_makefile
