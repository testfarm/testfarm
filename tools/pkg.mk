##
## Linux distribution packages builder
##

ifeq ($(VERSION),)
VERSION ?= $(shell $(dir $(lastword $(MAKEFILE_LIST)))/gitversion.sh)
endif
BUILDDATE ?= $(shell date +%y%m%d)

RPMARCH ?= $(shell arch)

DEBARCH ?= $(shell dpkg-architecture -qDEB_HOST_ARCH)
DEBNAME = $(PKGNAME)_$(VERSION)_$(DEBARCH).deb

MKDIR ?= mkdir -p

ifdef PKGNAME
check_pkg_vars:
ifdef PKGDIR
	$(MKDIR) $(PKGDIR)
else
	@echo "Variable PKGDIR not defined"; false
endif
ifndef DESTDIR
	@echo "Variable DESTDIR not defined"; false
endif

rpm: check check_pkg_vars install
	$(RM) $(DESTDIR)/DEBIAN
	$(MKDIR) $(PKGDIR)/BUILD
	sed -e 's/@NAME@/$(PKGNAME)/' \
	    -e 's/@VERSION@/$(SRCVERSION)/' \
	    -e 's/@RELEASE@/$(BUILDDATE)/' \
	    -e 's/@DEPS@/$(PKGDEPS)/' \
	    spec.in > $(PKGDIR)/RPM.spec
	find $(DESTDIR) -type f | sed 's|^$(DESTDIR)||' > $(PKGDIR)/BUILD/RPM.files
	echo "%_topdir $(PWD)/$(PKGDIR)" > $(HOME)/.rpmmacros
	rpmbuild -bb $(PKGDIR)/RPM.spec --buildroot=$(PWD)/$(DESTDIR) --target $(RPMARCH)
	$(MV) $(PKGDIR)/RPMS/*/*.rpm $(STAGING_DIR)

deb: check check_pkg_vars install
	$(MKDIR) $(DESTDIR)/DEBIAN
	for file in preinst postinst prerm postrm; do \
		[ -f $$file ] && install -m 755 $$file $(DESTDIR)/DEBIAN/; done; \
	sed -e 's/@NAME@/$(PKGNAME)/' \
	    -e 's/@ARCH@/$(DEBARCH)/' \
	    -e 's/@VERSION@/$(VERSION)/' \
	    -e 's/@DEPS@/$(PKGDEPS)/' \
	    control.in > $(DESTDIR)/DEBIAN/control
	fakeroot dpkg-deb --build $(DESTDIR) $(STAGING_DIR)/$(DEBNAME)
else
rpm deb:
	@echo "Variable PKGNAME not defined"; false
endif

pkgclean:
ifdef PKGDIR
	$(RM) $(PKGDIR)
else
	@true
endif
