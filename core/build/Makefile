##
## TestFarm - Test Suite Builder
##
## Author: Sylvain Giroudon
## Creation: 01-JUN-2006
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

CHECK_PACKAGES_deb += libgtk2-perl libxml-dom-perl libgtk2-gladexml-perl
CHECK_PACKAGES_rpm +=

include ../../tools/defs.mk

##### TARGETS
all:
	@true

##### Installation
install:
	@$(MKDIR) $(INSTALLDIR)/bin $(INSTALLDIR)/lib
	@$(CP) -v testfarm-edit.sh $(INSTALLDIR)/bin/
	@$(CP) -v testfarm-build.pl $(INSTALLDIR)/bin/testfarm-build
	@$(CP) -v testfarm-build.glade $(INSTALLDIR)/lib/
