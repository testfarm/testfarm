##
## TestFarm
## Test Output management
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
