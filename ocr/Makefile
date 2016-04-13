##
## TestFarm Virtual User -- OCR interfacing
##
## This file is part of TestFarm,
## the Test Automation Tool for Embedded Software.
## Please visit http://www.testfarm.org.
##
## TestFarm is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## TestFarm is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##

CHECK_PACKAGES_deb += quilt lzip help2man libnetpbm10-dev libtiff5-dev libglade2-dev libpng12-dev libreadline-dev libtesseract-dev
CHECK_PACKAGES_rpm += quilt lzip help2man netpbm-devel libtiff-devel libglade2-devel readline-devel tesseract-devel

include ../tools/defs.mk

##### $(BINS) and $(LIBS) DEPENDENCIES
all:
	git submodule update --init
	make -C cosi all

clean::
	make -C cosi clean

deb:
	make -C cosi deb DESTDIR=$(abspath $(DESTDIR)) PKGDIR=$(abspath $(PKGDIR))
