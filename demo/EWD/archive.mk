PKGNAME = testfarm-demo-ewd

DEBARCH = all
RPMARCH = noarch

SUBDIRS = $(dir $(wildcard */.tree))
FILES = $(wildcard *.tree *.wiz *.pl *.pm *.xml)

all: deb

include ../../tools/defs.mk

TARBALL = TestFarm-demo-EWD_$(VERSION).tgz

tarball: clean
	tar cfza $(TARBALL) $(SUBDIRS) $(FILES)

install: tarball
	mkdir -p $(INSTALLDIR)/demo
	cp -a $(TARBALL) $(INSTALLDIR)/demo/

clean::
	make clean
	rm -rf $(DESTDIR)
