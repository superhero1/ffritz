
CONFTYPE	= autoreconf

PKG_CONFIGURE_FLAGS = --with-ssl=mbedtls --with-stdout --with-pipe --with-alsa --with-tinysvcmdns LDFLAGS="-L$(SYSROOT)/lib -L$(SYSROOT)/usr/lib"

INSTALL_BIN=$(BUILDDIR)/shairport-sync

include ../../package.mk
