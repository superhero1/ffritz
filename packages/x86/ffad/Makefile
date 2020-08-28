
CONFTYPE=legacy

INSTALL_BIN  = $(BUILDDIR)/ffad

MAKE_OPTIONS = CASH_EXT_LIST="$(wildcard($(shell pwd)/ext/*))" CROSS_COMPILE=$(CROSS) tool

include ../../package.mk

all:	$(BUILDDIR)/ffad

$(BUILDDIR)/ffad: $(BUILDDIR)/cash
	cp $(BUILDDIR)/cash $(BUILDDIR)/ffad
      

