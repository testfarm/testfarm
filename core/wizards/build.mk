##
## TestFarm
## Test Suite Development Wizards - Top-level management Makefile
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

TREE_MK = /opt/testfarm/lib/tree.mk

.SUFFIXES: .tree .deps
.IGNORE: clean
.PHONY: clean cleantree cleandeps cleandoc checkrev

TREEDEPS = $(TARGETS:%=Makefile.d/%.tree.deps)
WIZDEPS = $(TARGETS:%=Makefile.d/%.wiz.deps)

.DEFAULT:
	@for i in $(TREEDEPS) $(WIZDEPS); do \
	  [ -f $$i ] && (cat $$i | xargs ls 2>&1 >/dev/null || /bin/rm -f $$i) || continue; \
	done

all: checkrev doc $(SYSTEMS) $(TARGETS)

REV = $(shell twiz-makefile -rev)
checkrev:
ifneq ($(REV),$(MAKEFILE_REV))
	$(warning The TestFarm Makefile Wizard has been updated to rev $(REV).)
	$(warning WARNING : Please run command 'twiz-makefile' to update workspace.)
else
	@true
endif

ifdef SYSTEMS
$(SYSTEMS):
	@echo '--- Generating local System Config library $@...'
	twiz-conf $<
endif

update: cleandeps deps

clean: cleantree cleandeps cleandoc
cleantree:
	@for TREENAME in $(TARGETS); do \
	  export TREENAME; make -r -f $(TREE_MK) clean; \
	done
cleandeps:
	/bin/rm -f Makefile.d/*.deps

deps: Makefile.d/wizdef.deps $(TREEDEPS)
	@for TREENAME in $(TARGETS); do \
	  export TREENAME; make -r -f $(TREE_MK) deps; \
	done

$(TARGETS):
	export TREENAME=$@; make -r -f $(TREE_MK)

%.pm: Makefile.d/wizdef.deps Makefile.d/$(TREENAME).tree.deps
	make -r -f $(TREE_MK) $@

$(TREEDEPS):
	@echo '---' Remaking $(<:%.tree=%) tree dependencies...
	twiz-deps -tree $< -wizlist $(@:%.tree.deps=%.wiz.deps) > $@

ifdef TESTFARM_WIZLIB
WIZLIB = $(subst :, ,$(TESTFARM_WIZLIB))
else
WIZLIB = wiz
endif
WIZFILES = $(wildcard $(WIZLIB:%=%/*.wizdef)) $(wildcard $(WIZLIB:%=%/*.pm))

Makefile.d/wizdef.deps: $(WIZLIB) $(WIZFILES)
	mkdir -p $(WIZLIB)
	@echo '---' Remaking .wizdef dependencies...
	@twiz-deps -wizdef $^ > $@

doc: wizdef.html wizdef.man

cleandoc:
	/bin/rm -f wizdef.html wizdef.man wizdef.pod

wizdef.html: Makefile.d/wizdef.deps
	@echo '---' Rebuilding wizdef HTML documentation
	twiz-pod -html | pod2html --title="WIZ Reference Manual" --outfile=$@
	/bin/rm -f pod2*.x~~ pod2*.tmp

wizdef.man: Makefile.d/wizdef.deps
	twiz-pod > wizdef.pod
	pod2man -d TestFarm -r "WIZ Reference Manual" -c "WIZ Reference Manual" wizdef.pod $@
	/bin/rm -f wizdef.pod

ifneq ($(strip $(TREENAME)),)
-include Makefile.d/wizdef.deps
-include Makefile.d/$(TREENAME).scripts.deps
endif
