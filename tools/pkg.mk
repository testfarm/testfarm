##
##    TestFarm -- Linux distribution packages builder
##
##    This file is part of TestFarm,
##    the Test Automation Tool for Embedded Software.
##    Please visit http://www.testfarm.org.
##
##    TestFarm is free software: you can redistribute it and/or modify
##    it under the terms of the GNU General Public License as published by
##    the Free Software Foundation, either version 3 of the License, or
##    (at your option) any later version.
##
##    TestFarm is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##    GNU General Public License for more details.
##
##    You should have received a copy of the GNU General Public License
##    along with TestFarm.  If not, see <http://www.gnu.org/licenses/>.
##

ifdef PKGNAME

ifeq ($(VERSION),)
VERSION ?= $(shell $(dir $(lastword $(MAKEFILE_LIST)))/gitversion.sh)
endif
BUILDDATE ?= $(shell date +%y%m%d)

RPMARCH ?= $(shell arch)

DEBARCH ?= $(shell dpkg-architecture -qDEB_HOST_ARCH)
DEBNAME = $(PKGNAME)_$(VERSION)_$(DEBARCH).deb

MKDIR ?= mkdir -p

check_pkg_vars:
ifdef PKGDIR
	$(MKDIR) $(PKGDIR) $(DELIVERY_DIR)
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
	$(MV) $(PKGDIR)/RPMS/*/*.rpm $(DELIVERY_DIR)/

deb: check check_pkg_vars install
	$(MKDIR) $(DESTDIR)/DEBIAN
	for file in preinst postinst prerm postrm; do \
		[ -f $$file ] && install -m 755 $$file $(DESTDIR)/DEBIAN/; done; \
	sed -e 's/@NAME@/$(PKGNAME)/' \
	    -e 's/@ARCH@/$(DEBARCH)/' \
	    -e 's/@VERSION@/$(VERSION)/' \
	    -e 's/@DEPS@/$(PKGDEPS)/' \
	    control.in > $(DESTDIR)/DEBIAN/control
	fakeroot dpkg-deb --build $(DESTDIR) $(DELIVERY_DIR)/$(DEBNAME)

endif  # PKGNAME

pkgclean:
ifdef PKGDIR
	$(RM) $(PKGDIR)
else
	@true
endif
