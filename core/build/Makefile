##
## TestFarm - Test Suite Builder
##
## Author: Sylvain Giroudon
## Creation: 01-JUN-2006
##

include ../../tools/defs.mk

PASTE = ../perl/paste.pl

##### TARGETS
PERLBINS = testfarm-build
SCRIPTS = testfarm-edit.sh

all: $(PERLBINS)

clean::
	$(RM) $(PERLBINS)

testfarm-build: testfarm-build.pl testfarm-build.glade
	$(PASTE) '__GLADE_INTERFACE__' $^ > $@
	chmod +x $@

##### Installation
install_scripts: $(SCRIPTS)
	@$(CP) -v $^ $(INSTALLDIR)/bin/

install_perlbins: $(PERLBINS)
	@$(MKDIR) $(INSTALLDIR)/bin
	@$(CP) -v $^ $(INSTALLDIR)/bin/

install: install_scripts install_perlbins
