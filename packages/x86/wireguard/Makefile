
CONFTYPE	= legacy
MAKE_SUBDIR	= /src
#CUSTOM_DEP	= modules

INSTALL_LIB	= build/src/wireguard.ko

ifeq ($(KERNELDIR),)
KERNELDIR      = $(shell cd ../modules_3.12.74/build; pwd)
endif
KERNELRELEASE  = $(shell cat $(KERNELDIR)/include/config/kernel.release)

MAKE_OPTIONS    = KERNELDIR=$(KERNELDIR) KERNELRELEASE=$(KERNELRELEASE) CC=$(CC) LD=$(CROSS)ld WITH_WGQUICK=yes

include ../../package.mk
