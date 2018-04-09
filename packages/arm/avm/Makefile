# This Makefile unpacks the source tarball from avm for the
# puma6/ARM CPU.
#
#
# Targets:
#   default	    - Build toolchain and some tools 
#   kernel	    - Build kernel
#   kernel-modules  - Build kernel modules
#   kernel-config   - Configure kernel (optional)
#   toolchain	    - Build toolchain
#

TOPDIR	    = $(shell pwd)
DESTDIR	    = $(TOPDIR)/output


BUILDROOT   = $(TOPDIR)/build/prefix_sehr_sehr_sehr_sehr_sehr_sehr_sehr_sehr_sehr_lang_buildroot-2013.05
TOOLCHAIN   = $(BUILDROOT)/output/host/usr/bin

URLFILE = ../../url-avm-toolchain-arm

URL	= $(shell cat $(URLFILE))
FILE	= ../../dl/$(shell basename $(URL))

CURL	= $(BUILDROOT)/output/dl/curl-7.28.1.tar.bz2


## _CROSS is the cross compiler used. By default we use our own, but if required
## the avm toolchain can be used as well.
#
#_CROSS = $(TOPDIR)/build/prefix_sehr_sehr_sehr_sehr_sehr_sehr_sehr_sehr_sehr_lang_buildroot-2013.02/output/host/usr/bin/i686-buildroot-linux-uclibc- 
_CROSS = $(TOOLCHAIN)/armeb-buildroot-linux-uclibcgnueabi-

## Parameters for kernel build (based on build/kernel/gpl_compile_kernel.sh)
#
export FRITZ_BOX_BUILD_DIR  := $(TOPDIR)/build/kernel/
export _KERNEL_BUILD	    := linux-2.6.39.3
export KERNEL_BUILD_DIR	    := $(FRITZ_BOX_BUILD_DIR)/$(_KERNEL_BUILD)
export KERNEL_CONFIG	    := $(KERNEL_BUILD_DIR)/.config
export INSTALL_MOD_PATH	    := $(KERNEL_BUILD_DIR)/filesystem


#########################################################################

.PHONY:	kernel

all:	toolchain
	make -C $(BUILDROOT) readline tcpdump tcpreplay curl rsync 

toolchain:  toolchain.stamp

toolchain.stamp: avm-prep.stamp 
	make -C build KERNEL_LAYOUT=armp6  HOSTCC=gcc
	rm -rf GPL
	touch toolchain.stamp

kernel-config: avm-prep.stamp
	make -C build KERNEL_LAYOUT=armp6  HOSTCC=gcc build_root_unpack
	make -C $(KERNEL_BUILD_DIR) ARCH=arm menuconfig CROSS_COMPILE=$(_CROSS)

kernel: avm-prep.stamp
	make -C build KERNEL_LAYOUT=armp6  HOSTCC=gcc build_root_unpack
	make -C $(KERNEL_BUILD_DIR) ARCH=arm CROSS_COMPILE=$(_CROSS) -j8

kernel-modules: avm-prep.stamp
	make -C build KERNEL_LAYOUT=armp6  HOSTCC=gcc build_root_unpack
	make -C $(KERNEL_BUILD_DIR) ARCH=arm CROSS_COMPILE=$(_CROSS) -j8 modules

$(FILE): $(URLFILE)
	cd ../../dl; wget -N $(URL)
	touch $(FILE)

avm-prep.stamp: $(FILE) $(CURL)
	tar xf $(FILE) GPL/GPL-release_kernel.tar.gz GPL/GPL-gcc.tar.gz
	mkdir -p build
	cd build; tar xf $(TOPDIR)/GPL/GPL-gcc.tar.gz
	mv build/kernel-2.6.39.3.patch build/kernel-2.6.39.3.patch_XXX_does_not_apply
	mkdir -p kernel
	cd kernel; tar xf $(TOPDIR)/GPL/GPL-release_kernel.tar.gz
	make -C kernel/linux-2.6.39/ ARCH=arm oldconfig
	make -C kernel/linux-2.6.39/ ARCH=arm prepare || true
	touch avm-prep.stamp

$(CURL):
	mkdir -p `dirname $(CURL)`
	wget --no-check-certificate http://curl.haxx.se/download/curl-7.28.1.tar.bz2 -O $(CURL)

ff-modules: avm-prep.stamp
	make -C build KERNEL_LAYOUT=armp6  HOSTCC=gcc build_root_unpack
	make -C $(KERNEL_BUILD_DIR) ARCH=arm CROSS_COMPILE=$(_CROSS) -j8 modules 

ff-modules-install:	ff-modules
	cp $(KERNEL_BUILD_DIR)/drivers/usb/class/cdc-acm.ko $(DESTDIR)

clean:
	rm -rf build kernel GPL
	rm -f *.stamp

distclean:
	rm -rf build
	rm -f $(FILE)
