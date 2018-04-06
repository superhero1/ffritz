# This Makefile unpacks the source tarball from avm for the
# puma6/Atom CPU.
# Although it's possible to compile the buildroot toolchain it's actually
# not used.
#
# The main goal here is to be able to compile matching kernel modules.
#
#
# Targets:
#   default	    - Just unpacks the tarballs
#   kernel	    - Build kernel
#   kernel-modules  - Build kernel modules
#   kernel-config   - Configure kernel (optional)
#   toolchain	    - Build toolchain
#

TOPDIR	    = $(shell pwd)
DESTDIR	    = $(TOPDIR)/output


BUILDROOT   = $(shell cd ../buildroot/build; pwd)
TOOLCHAIN   = $(shell cd $(BUILDROOT)/output/host/usr/bin/; pwd)

URLFILE = ../../url-avm-toolchain-atom

URL	= $(shell cat $(URLFILE))
FILE	= ../../dl/$(shell basename $(URL))

## _CROSS is the cross compiler used. By default we use our own, but if required
## the avm toolchain can be used as well.
#
#_CROSS = $(TOPDIR)/build/prefix_sehr_sehr_sehr_sehr_sehr_sehr_sehr_sehr_sehr_lang_buildroot-2013.02/output/host/usr/bin/i686-buildroot-linux-uclibc- 
_CROSS = $(TOOLCHAIN)/i686-buildroot-linux-uclibc-

## Parameters for kernel build (based on build/kernel/gpl_compile_kernel.sh)
#
export FRITZ_BOX_BUILD_DIR  := $(TOPDIR)/build/kernel/
export _KERNEL_BUILD	    := linux-2.6.39.3
export KERNEL_BUILD_DIR	    := $(FRITZ_BOX_BUILD_DIR)/$(_KERNEL_BUILD)
export KERNEL_CONFIG	    := $(KERNEL_BUILD_DIR)/.config
export INSTALL_MOD_PATH	    := $(KERNEL_BUILD_DIR)/filesystem


#########################################################################

.PHONY:	kernel

all:	avm-prep.stamp

toolchain:  toolchain.stamp

toolchain.stamp: avm-prep.stamp 
	make -C build KERNEL_LAYOUT=x86  HOSTCC=gcc-4.7
	rm -rf GPL
	touch toolchain.stamp

kernel-config: avm-prep.stamp
	make -C build KERNEL_LAYOUT=x86  HOSTCC=gcc-4.7 build_root_unpack
	make -C $(KERNEL_BUILD_DIR) ARCH=x86 menuconfig CROSS_COMPILE=$(_CROSS)

kernel: avm-prep.stamp
	make -C build KERNEL_LAYOUT=x86  HOSTCC=gcc-4.7 build_root_unpack
	make -C $(KERNEL_BUILD_DIR) ARCH=x86 CROSS_COMPILE=$(_CROSS) -j8

kernel-modules: avm-prep.stamp
	make -C build KERNEL_LAYOUT=x86  HOSTCC=gcc-4.7 build_root_unpack
	make -C $(KERNEL_BUILD_DIR) ARCH=x86 CROSS_COMPILE=$(_CROSS) -j8 modules

$(FILE): $(URLFILE)
	cd ../../dl; wget $(URL)
	touch $(FILE)

avm-prep.stamp: $(FILE)
	tar xf $(FILE) GPL/GPL-release_kernel.tar.gz GPL/GPL-gcc.tar.gz
	mkdir -p build
	cd build; tar xf $(TOPDIR)/GPL/GPL-gcc.tar.gz
	mv build/kernel-2.6.39.3.patch build/kernel-2.6.39.3.patch_XXX_does_not_apply
	cp buildroot.config.x86 build
	mkdir -p kernel
	cd kernel; tar xf $(TOPDIR)/GPL/GPL-release_kernel.tar.gz
	touch avm-prep.stamp

ff-modules: avm-prep.stamp
	make -C build KERNEL_LAYOUT=x86  HOSTCC=gcc-4.7 build_root_unpack
	cp mod/kernel_config $(KERNEL_BUILD_DIR)/.config
	make -C $(KERNEL_BUILD_DIR) ARCH=x86 CROSS_COMPILE=$(_CROSS) -j8 modules

ff-modules-install:	ff-modules
	cp $(KERNEL_BUILD_DIR)/drivers/usb/class/cdc-acm.ko $(DESTDIR)

clean:
	rm -rf build
	rm -f *.stamp

distclean:
	rm -rf build
	rm -f $(FILE)
