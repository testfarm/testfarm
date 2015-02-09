##### Platform environment
GLIB = glib-2.0
GMODULE = gmodule-2.0
GTHREAD = gthread-2.0

##### Compiler options
CFLAGS += -O6 -mmmx -msse

##### Install directories
PERLLIBDIR = $(INSTALLDIR)/lib/TestFarm
MODLIBDIR = $(INSTALLDIR)/lib/vu
USRLIBDIR = $(INSTALLROOT)/usr/lib/testfarm-vu

install_modules:
	$(MKDIR) $(MODLIBDIR)
	$(CP) $^ $(MODLIBDIR)
