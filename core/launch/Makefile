##
## TestFarm - Launch Pad and Service Starter
##
## Author: Sylvain Giroudon
## Creation: 07-JUN-2004
##

include ../../tools/defs.mk

PASTE = ../perl/paste.pl

##### OPTIONS
LDFLAGS += -lcrypto -Xlinker -s -llicense -linstall

CFLAGS  += $(shell pkg-config --cflags $(GTK))
LDFLAGS += $(shell pkg-config --libs $(GTK))

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
