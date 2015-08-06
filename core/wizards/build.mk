##
## TestFarm
## Test Suite Development Wizards
## Top-level management Makefile
##
## $Revision: 1235 $
## $Date: 2013-06-28 14:37:43 +0200 (ven., 28 juin 2013) $
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
