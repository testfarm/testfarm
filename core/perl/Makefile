##
## TestFarm - Perl Modules
##
## Author: Sylvain Giroudon
## Creation: 14-JUL-2006
##

CHECK_PACKAGES_deb += libsys-gamin-perl
CHECK_PACKAGES_rpm +=

include ../../tools/defs.mk

##### TARGETS
HEADERS = cryptoperl.c
PERLLIBS = Version.pm Dialog.pm FAM.pm Locate.pm Config.pm Service.pm \
	   Engine.pm Trig.pm Exec.pm Env.pm \
	   Interface.pm ../../interface/link/InterfaceInput.pm \
	   Serial.pm
LIBDIR = $(INSTALLDIR)/lib/TestFarm

all: $(PERLLIBS)

clean::
	$(RM) Version.pm Dialog.pm

##### PERL libraries
Version.pm: Version.in.pm
	sed 's/@VERSION@/$(SRCVERSION)/g' $< >$@
Dialog.pm: Dialog.in.pm TDialog.glade
	./paste.pl __GLADE_INTERFACE__ $^ > $@

##### Installation
install_perllibs: $(PERLLIBS)
	@$(MKDIR) $(LIBDIR)
	@$(CP) -v $^ $(LIBDIR)

.PHONY: install
install: install_perllibs
