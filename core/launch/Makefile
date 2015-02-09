##
## TestFarm - Launch Pad and Service Starter
##
## Author: Sylvain Giroudon
## Creation: 07-JUN-2004
##


CHECK_PACKAGES_deb += libgtk2-perl libxml-dom-perl libgtk2-gladexml-perl
CHECK_PACKAGES_rpm +=

include ../../tools/defs.mk

PASTE = ../perl/paste.pl

##### TARGETS
PERLBINS = testfarm-launch

all: $(PERLBINS)

clean::
	$(RM) $(PERLBINS)

testfarm-launch: testfarm-launch.pl testfarm-launch.glade2
	$(PASTE) '__GLADE_INTERFACE__' $^ > $@
	chmod +x $@

##### Installation
install_perlbins: $(PERLBINS)
	@$(MKDIR) $(INSTALLDIR)/bin
	@$(CP) -v $^ $(INSTALLDIR)/bin/
	@$(CP) -v testfarm-service.pl $(INSTALLDIR)/bin/testfarm-service

install: install_perlbins
