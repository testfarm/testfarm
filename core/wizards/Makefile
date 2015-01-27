##
## TestFarm - Test Suite Development Wizards
##

include ../../tools/defs.mk

##### TARGETS
PERLLIBDIR = $(INSTALLDIR)/lib/TestFarm
PERLLIBS = Wiz.pm
PERLBINS = twiz-conf twiz-script twiz-check twiz-makefile twiz-deps twiz-pod
MANUALS = twiz-script.1

all: $(MANUALS)

clean::
	$(RM) $(MANUALS)

##### Manuals
twiz-script.1: twiz-script.pl

%.pl %.1:
	pod2man $^ > $@

##### Installation
install_perlbins:
	@$(MKDIR) $(INSTALLDIR)/bin
	for bin in $(PERLBINS); do \
	  $(CP) -v $$bin.pl $(INSTALLDIR)/bin/$$bin ;\
	done

install_perlibs: $(PERLLIBS)
	@$(MKDIR) $(PERLLIBDIR)
	@$(CP) -v $^ $(PERLLIBDIR)

install_man: $(MANUALS)
	@$(MKDIR) $(INSTALLDIR)/man/man1
	@$(CP) -v $^ $(INSTALLDIR)/man/man1

install_xml: config.dtd config.xsl
	@$(MKDIR) $(INSTALLDIR)/lib
	@$(CP) -v $^ $(INSTALLDIR)/lib

install_makefile: build.mk tree.mk script.mk
	@$(MKDIR) $(INSTALLDIR)/lib
	@$(CP) -v $^ $(INSTALLDIR)/lib

install: install_perlibs install_perlbins install_xml install_man install_makefile
