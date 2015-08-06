PKGNAME = testfarm-demo-ewd

DEBARCH = all
RPMARCH = noarch

SUBDIRS = $(dir $(wildcard */.tree))
FILES = $(wildcard *.tree *.wiz *.pl *.pm *.xml)

PKGDIR := ../../tmp/demo
DESTDIR := $(PKGDIR)/root
INSTALLDIR := $(DESTDIR)/opt/testfarm/demo

include ../../tools/pkg.mk

TARBALL = TestFarm-demo-EWD_$(VERSION).tgz

tarball: clean
	tar cfza $(TARBALL) $(SUBDIRS) $(FILES)

install: tarball
	mkdir -p $(INSTALLDIR)
	cp -a $(TARBALL) $(INSTALLDIR)/

clean:
	make clean
	rm -rf $(DESTDIR)

check:
	@true
