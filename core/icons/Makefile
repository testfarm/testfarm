##
## TestFarm - Icons and Logos
##
## Author: Sylvain Giroudon
## Creation: 18-AUG-2005
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
ICONS = go.png step.png edit.png list.png log.png view.png \
	passed.png failed.png inconclusive.png skip.png \
	start.png stop.png breakpoint.png break.png abort.png \
	unavailable.png delivered.png reviewed.png approved.png \
	modified.png updated.png ignore.png \
	script-16.png script-32.png \
	launch-16.png launch-32.png \
	node_tree.png node_seq.png node_case.png node_unknown.png \
	error.png info.png panic.png warning.png \
	file.png entry.png output.png report.png \
	testfarm.png testfarm-logo.png

ICONDIR = $(INSTALLDIR)/icons

##### INSTALL RULES
install_icons: $(ICONS)
	$(MKDIR) $(ICONDIR)
	$(CP) -v $^ $(ICONDIR)

install: install_icons
