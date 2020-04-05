
CONFTYPE	= legacy
MAKE_SUBDIR	= /src

include ../../package.mk

INSTALL_BIN	= $(BUILDDIR)/src/wg $(BUILDDIR)/src/wg-quick/wg-quick

MAKE_OPTIONS    = CC=$(CC) LD=$(CROSS)ld WITH_WGQUICK=yes

$(BUILDDIR)/src/wg-quick/wg-quick:	$(BUILDDIR)/src/wg-quick/linux.bash
		cp -f $< $@

all:		$(BUILDDIR)/src/wg-quick/wg-quick
