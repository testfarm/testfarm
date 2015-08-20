PKGNAME = testfarm-demo-ewd

DEBARCH = all
RPMARCH = noarch

SUBDIRS = $(dir $(wildcard */.tree)) wiz objects
FILES = $(wildcard *.tree *.wiz *.pl *.pm *.xml) passwd

all: deb

include ../../tools/defs.mk

TARBALL = TestFarm-demo-EWD_$(VERSION).tgz

tarball: clean
	tar cfza $(TARBALL) $(SUBDIRS) $(FILES)

install: tarball
	mkdir -p $(INSTALLDIR)/demo
	cp -a $(TARBALL) $(INSTALLDIR)/demo/
	install -m 755 install.sh $(INSTALLDIR)/demo/

clean::
	-make clean
	rm -rf $(DESTDIR)
