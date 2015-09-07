##
##    TestFarm -- Linux development packages checker
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

ifneq ($(shell which rpm 2>/dev/null),)
ifneq ($(shell rpm -q fedora-release | grep ^fedora-release),)
HOST_DISTRO = Fedora
else ifneq ($(shell rpm -q redhat-release | grep ^redhat-release),)
HOST_DISTRO = RedHat
else ifneq ($(shell rpm -q centos-release | grep ^centos-release),)
HOST_DISTRO = CentOS
endif
ifdef HOST_DISTRO
PKGSUFFIX = rpm
PKGTOOL = rpm -q
CHECK_PACKAGES += $(CHECK_PACKAGES_rpm)
CHECK_PACKAGES += gcc gcc-c++ rpm-build
endif
endif

ifndef HOST_DISTRO
ifneq ($(shell which dpkg 2>/dev/null),)
HOST_DISTRO = Debian/Ubuntu
PKGSUFFIX = deb
PKGTOOL = dpkg -s
CHECK_PACKAGES += $(CHECK_PACKAGES_deb)
CHECK_PACKAGES += gcc g++ fakeroot dpkg-dev
endif
endif

check::
ifneq ($(HOST_DISTRO),)
	@echo -n "Checking required packages ($(HOST_DISTRO))... "
	@MISSING=""; \
	for pkg in $(CHECK_PACKAGES); do \
	  $(PKGTOOL) $$pkg >/dev/null 2>/dev/null || MISSING="$$MISSING $$pkg"; \
	done; \
	if [ -n "$$MISSING" ]; then \
	  /bin/echo -e "\nERROR: missing packages: $$MISSING"; \
	  false; \
	else \
	  echo "OK"; \
	fi
else
	@echo "No package manager found"
endif
