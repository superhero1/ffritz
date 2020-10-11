
# Find directory containing package.mk upwards
#
PKGDIR=$(shell while test -f package.mk && echo $$PWD && exit 0; [ $$PWD != / ]; do cd ..; done)

CONFTYPE            = 
PKG_CONFIGURE_FLAGS =
INSTALL_BIN         = 
INST_TO_TOOLCHAIN   = 1

include $(PKGDIR)/package.mk
