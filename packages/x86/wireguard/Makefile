
CONFTYPE	= legacy
MAKE_SUBDIR	= /src
#CUSTOM_DEP	= modules

INSTALL_LIB	= build/src/wireguard.ko
INSTALL_BIN	= build/src/tools/wg build/src/tools/wg-quick/wg-quick

ifeq ($(KERNELDIR),)
KERNELDIR      = $(shell cd ../modules_3.12.74/build; pwd)
endif
KERNELRELEASE  = $(shell cat $(KERNELDIR)/include/config/kernel.release)

MAKE_OPTIONS    = KERNELDIR=$(KERNELDIR) KERNELRELEASE=$(KERNELRELEASE) CC=$(CC) LD=$(CROSS)ld WITH_WGQUICK=yes

all:

build/src/tools/wg-quick/wg-quick:	build/src/tools/wg-quick/linux.bash
		cp -f $< $@

install:	all build/src/tools/wg-quick/wg-quick

include ../../package.mk
