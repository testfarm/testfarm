##
## TestFarm
## Miscellaneous Useful Stuffs
##
## Author: Sylvain Giroudon
## Creation: 17-AUG-1999
##

GLIB = glib-2.0

LIBS = libuseful.a

BINS = child_test
ifdef GLIB
BINS += filelist_test
endif

include ../../tools/defs.mk

ifdef GLIB
CFLAGS += -DENABLE_GLIB
endif

##### TARGETS
HEADERS = useful.h pty.h child.h sig.h
ifdef GLIB
HEADERS += filelist.h
endif

##### SOURCES
LIBSRCS = strread.c strcleanup.c strskip.c strupper.c strtimestamp.c strxml.c \
	  sleep_ms.c sig.c \
	  pty.c fgets2.c \
	  child.c child_test.c
ifdef GLIB
LIBSRCS += filelist.c
endif
ALLCSRCS = $(LIBSRCS)

##### RULES
libuseful.a: $(LIBSRCS:%.c=%.o)

child_test: child_test.o child.o
filelist_test: filelist_test.o filelist.o

install:
	@echo >/dev/null

-include .deps
