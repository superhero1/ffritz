
CONFTYPE	= legacy
MAKE_SUBDIR	= /src
#CUSTOM_DEP	= modules

INSTALL_LIB	= $(BUILDDIR)/src/wireguard.ko

ifeq ($(KERNELDIR),)
KERNELDIR      = $(shell cd ../kernel_4.9.175/build; pwd)
endif
KERNELRELEASE  = $(shell cat $(KERNELDIR)/include/config/kernel.release)

MAKE_OPTIONS    = KERNELDIR=$(KERNELDIR) KERNELRELEASE=$(KERNELRELEASE) CC=$(CC) LD=$(CROSS)ld WITH_WGQUICK=yes

include ../../package.mk
