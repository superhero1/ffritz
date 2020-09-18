
CONFTYPE=legacy

INSTALL_BIN  = $(BUILDDIR)/ffad

MAKE_OPTIONS = CROSS_COMPILE=$(CROSS) tool

CUSTOM_DEP   = $(BUILDDIR)/ext/ffad

include ../../package.mk

all:	$(BUILDDIR)/ffad 

$(BUILDDIR)/ffad:
	cp $(BUILDDIR)/cash $(BUILDDIR)/ffad

$(BUILDDIR)/ext/ffad:
	@test -d $@ || (mkdir -p $@ && cp -va ext/ffad $(BUILDDIR)/ext)
