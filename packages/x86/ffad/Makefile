
CONFTYPE=legacy

INSTALL_BIN  = $(BUILDDIR)/ffad

MAKE_OPTIONS = CROSS_COMPILE=$(CROSS) CASH_EXT_LIST=$(TOPDIR)/ext/ffad BISON=/usr/bin/bison tool

CUSTOM_DEP   = $(BUILDDIR)/ext/ffad

include ../../package.mk

all:	$(BUILDDIR)/ffad 

$(BUILDDIR)/ffad:
	cp $(BUILDDIR)/cash $(BUILDDIR)/ffad

