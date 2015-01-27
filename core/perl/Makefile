##
## TestFarm - Perl Modules
##
## Author: Sylvain Giroudon
## Creation: 14-JUL-2006
##

include ../../tools/defs.mk

##### TARGETS
HEADERS = cryptoperl.c
PERLLIBS = Dialog.pm FAM.pm Locate.pm Config.pm Service.pm \
	   Engine.pm Trig.pm Exec.pm Env.pm \
	   Interface.pm ../../interface/link/InterfaceInput.pm \
	   Serial.pm
LIBDIR = $(INSTALLDIR)/lib/TestFarm

all: $(PERLLIBS)

clean::
	$(RM) Dialog.pm

##### PERL libraries
Dialog.pm: Dialog.in.pm TDialog.glade
	./paste.pl __GLADE_INTERFACE__ $^ > $@

##### Installation
install_perllibs: $(PERLLIBS)
	@$(MKDIR) $(LIBDIR)
	@$(CP) -v $^ $(LIBDIR)

.PHONY: install
install: install_perllibs
