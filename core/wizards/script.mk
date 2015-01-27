##
## TestFarm
## Test Suite Development Wizards
## Test Script management Makefile
##
## $Revision: 1238 $
## $Date: 2013-08-05 17:46:19 +0200 (lun., 05 août 2013) $
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
