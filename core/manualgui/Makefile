##
## TestFarm - Manual User Interface
##

GLIB = glib-2.0

include ../../tools/defs.mk

##### TARGETS
PERLBINS = testfarm-manual testfarm-manual-interface

##### $(BINS) and $(LIBS) DEPENDENCIES
install_perlbins:
	@$(MKDIR) $(INSTALLDIR)/bin
	for bin in $(PERLBINS); do \
	  $(CP) -v $$bin.pl $(INSTALLDIR)/bin/$$bin ;\
	done

install: install_perlbins
