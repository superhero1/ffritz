
CONFTYPE	= legacy
MAKE_SUBDIR	= /src
CUSTOM_DEP	= modules

INSTALL_LIB	= build/src/wireguard.ko

MAKE_OPTIONS    = -C ../../../modules/build M=$(TOPDIR)/build/src LD=$(CROSS)ld

include ../../package.mk

modules:
	make -C ../modules

