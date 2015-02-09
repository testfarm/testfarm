##
## TestFarm Virtual User -- OCR interfacing
##
## $Revision: 1150 $
## $Date: 2010-06-04 23:13:45 +0200 (ven., 04 juin 2010) $
##

include ../../tools/defs.mk
include ../defs.mk

##### TARGETS
LAUNCHERS = gocr.sh tesseract.sh ocrad.sh

OCRLIBDIR = $(USRLIBDIR)/ocr
MODLIBDIR = $(OCRLIBDIR)

##### $(BINS) and $(LIBS) DEPENDENCIES
all:
	@echo >/dev/null

install_launchers: $(LAUNCHERS)
	$(MKDIR) $(OCRLIBDIR)
	$(CP) $^ $(OCRLIBDIR)

install: install_launchers
