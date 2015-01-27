#****************************************************************************
#* TestFarm                                                                 *
#* Report Configuration and Generation                                      *
#****************************************************************************
#* Author: Sylvain Giroudon                                                 *
#* Creation: 30-MAR-2005                                                    *
#****************************************************************************

GTK = gtk+-2.0

CHECK_PACKAGES_deb += libxml2-dev libssl-dev
CHECK_PACKAGES_rpm += libxml2-devel openssl-devel

LIBS = libreport.a
BINS = testfarm-config testfarm-report

include ../../tools/defs.mk

##### OPTIONS
LIBDIRS = interface/useful core/output core/codegen core/report core/config

CFLAGS += $(LIBDIRS:%=-I../../%) -I./$(GTK)
LDFLAGS += $(LIBDIRS:%=-L../../%) -loutput -lcodegen -luseful -lreport -linstall

CFLAGS  += $(shell xml2-config --cflags)
LDFLAGS += $(shell xml2-config --libs)

CFLAGS  += $(shell pkg-config --cflags libcrypto)
LDFLAGS += $(shell pkg-config --libs libcrypto)

##### TARGETS
HEADERS = report_config.h

##### SOURCES
REPORT_LIB_SRCS = report_config.c report_elapsed.c report_log.c report_xslt.c
OUTPUT_LIB_SRCS = validate.c md5.c output.c output_tree.c output_node.c output_info.c output_stat.c \
		  output_file.c output_xml_load.c output_xml.c
GLADE_SRCS = $(GTK)/support.c $(GTK)/interface.c
CONFIG_SRCS = testfarm-config.c report_config_gui.c
REPORT_SRCS = testfarm-report.c
ALLCSRCS = $(REPORT_LIB_SRCS) $(CONFIG_SRCS) $(REPORT_SRCS) $(GLADE_SRCS:%=$(GTK)/%)

##### $(BINS) and $(LIBS) DEPENDENCIES
libreport.a: $(REPORT_LIB_SRCS:%.c=%.o)
testfarm-config: $(CONFIG_SRCS:%.c=%.o) $(GLADE_SRCS:%.c=%.o)
testfarm-report: $(REPORT_SRCS:%.c=%.o)

.PHONY: install
install: install_bin

clean::
	$(RM) $(GTK)/*.o

-include .deps
