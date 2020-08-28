
CONFTYPE=legacy

INSTALL_BIN  = $(BUILDDIR)/cash
INSTALL_BIN += $(BUILDDIR)/cash_wrap
INSTALL_BIN += $(BUILDDIR)/cash_wrap.so

MAKE_OPTIONS = CASH_EXT_LIST="$(wildcard($(shell pwd)/ext/*))" CROSS_COMPILE=$(CROSS)

include ../../package.mk
