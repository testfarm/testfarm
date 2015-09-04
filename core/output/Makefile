##
## TestFarm
## Test Output management
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
## You should have received a copy of the GNU General Public License
## along with TestFarm.  If not, see <http://www.gnu.org/licenses/>.
##

GLIB = glib-2.0

CHECK_PACKAGES_deb += libxml2-dev libssl-dev
CHECK_PACKAGES_rpm += libxml2-devel openssl-devel

LIBS = liboutput.a

include ../../tools/defs.mk

##### OPTIONS
LIBDIRS = core/codegen interface/useful core/config

CFLAGS += $(LIBDIRS:%=-I../../%)
LDFLAGS += $(LIBDIRS:%=-L../../%) -loutput -lcodegen -luseful -linstall

CFLAGS  += $(shell xml2-config --cflags)
LDFLAGS += $(shell xml2-config --libs)

CFLAGS  += $(shell pkg-config --cflags libcrypto)
LDFLAGS += $(shell pkg-config --libs libcrypto)

##### TARGETS
HEADERS = validate.h md5.h \
	  output.h output_tree.h output_node.h output_info.h output_stat.h \
	  output_file.h output_xml.h output_xml_load.h

##### SOURCES
OUTPUT_LIB_SRCS = validate.c md5.c \
		  output.c output_tree.c output_node.c output_info.c output_stat.c \
		  output_file.c output_xml_load.c output_xml.c
ALLCSRCS = $(OUTPUT_LIB_SRCS)

##### $(BINS) and $(LIBS) DEPENDENCIES
liboutput.a: $(OUTPUT_LIB_SRCS:%.c=%.o)

install_xml: output.dtd output.xsl text.xsl \
	     styles.xml styles.xsl \
	     title.xml title.xsl \
	     verdict.xml verdict.xsl \
	     criticity.xml criticity.xsl \
	     validate.xml validate.xsl \
	     report.xml report.xsl
	@$(MKDIR) $(INSTALLDIR)/lib
	@$(CP) -v $^ $(INSTALLDIR)/lib

.PHONY: install
install: install_xml

-include .deps
