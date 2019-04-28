BUILDROOT_TARGETS   = 
BUILDROOT_AVM	    = 1

include ../../buildroot.mk

## This package can't be downloaded by buildroot properly
##
PKGCONF_URL	= $(shell cat ../../url-pkgconf)
PKGCONF		= $(shell basename $(PKGCONF_URL))

arch-patches:
	cp mod/ffritz_arm_defconfig build/configs
	make -C build ffritz_arm_defconfig
