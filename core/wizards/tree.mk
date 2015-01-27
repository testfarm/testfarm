##
## TestFarm
## Test Suite Development Wizards
## Test Tree management Makefile
##
## $Revision: 286 $
## $Date: 2006-11-16 11:23:00 +0100 (jeu., 16 nov. 2006) $
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
