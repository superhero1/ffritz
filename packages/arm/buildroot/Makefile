BUILDROOT_TARGETS   = 
BUILDROOT_AVM	    = 1

include ../../buildroot.mk

## This package can't be downloaded by buildroot properly
##
PKGCONF_URL	= $(shell cat ../../url-pkgconf)
PKGCONF		= $(shell basename $(PKGCONF_URL))

arch-patches:
	@diff -qN mod/config build/.config || cp mod/config build/.config
