BUILDROOT_TARGETS   = 
BASE_TARGETS   = openssl

include ../../buildroot.mk

## This package can't be downloaded by buildroot properly
##
PKGCONF_URL	= $(shell cat ../../url-pkgconf)
PKGCONF		= $(shell basename $(PKGCONF_URL))

arch-patches:	.gcc.patch.applied
	@if [ ! -f $(DLDIR)/$(PKGCONF) ]; then wget -O $(DLDIR)/$(PKGCONF) $(PKGCONF_URL); fi
	@diff -qN mod/config build/.config || cp mod/config build/.config
#	@diff -qN mod/uClibc-0.9.33.config build/toolchain/uClibc/uClibc-0.9.33.config || cp mod/uClibc-0.9.33.config build/toolchain/uClibc/uClibc-0.9.33.config
#	@diff -qN mod/libusb.mk build/package/libusb/libusb.mk || cp mod/libusb.mk build/package/libusb/libusb.mk
	@diff -qN mod/busybox-config build/package/busybox/busybox.config || cp mod/busybox-config build/package/busybox/busybox.config
#	@diff -qN mod/gmp.mk build/package/gmp/gmp.mk || cp mod/gmp.mk build/package/gmp/gmp.mk
#	@diff -qN mod/ltrace.mk build/package/ltrace/ltrace.mk || cp mod/ltrace.mk build/package/ltrace/ltrace.mk
	
.gcc.patch.applied:
	tar xf gcc-4.9.3.tar
	touch $@
