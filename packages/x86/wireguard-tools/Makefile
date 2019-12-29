
CONFTYPE	= legacy
MAKE_SUBDIR	= /src

INSTALL_BIN	= build/src/wg build/src/wg-quick/wg-quick

MAKE_OPTIONS    = CC=$(CC) LD=$(CROSS)ld WITH_WGQUICK=yes

all:

build/src/wg-quick/wg-quick:	build/src/wg-quick/linux.bash
		cp -f $< $@

install:	all build/src/wg-quick/wg-quick

include ../../package.mk
