##
## Linux development packages checker
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
