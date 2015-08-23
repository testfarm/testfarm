##
## TestFarm - Launch Pad and Service Starter
##
## Author: Sylvain Giroudon
## Creation: 07-JUN-2004
##


CHECK_PACKAGES_deb += libgtk2-perl libxml-dom-perl libgtk2-gladexml-perl
CHECK_PACKAGES_rpm +=

include ../../tools/defs.mk

##### TARGETS
all:
	@true

##### Installation
install:
	@$(MKDIR) $(INSTALLDIR)/bin $(INSTALLDIR)/lib
	@$(CP) -v testfarm-launch.pl $(INSTALLDIR)/bin/testfarm-launch
	@$(CP) -v testfarm-service.pl $(INSTALLDIR)/bin/testfarm-service
	@$(CP) -v testfarm-launch.glade2 $(INSTALLDIR)/lib/
