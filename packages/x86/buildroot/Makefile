BUILDROOT_TARGETS   = 
BASE_TARGETS   = openssl

include ../../buildroot.mk

## This package can't be downloaded by buildroot properly
##
PKGCONF_URL	= $(shell cat ../../url-pkgconf)
PKGCONF		= $(shell basename $(PKGCONF_URL))

arch-patches:	.patch.applied
	@if [ ! -f $(DLDIR)/$(PKGCONF) ]; then wget -O $(DLDIR)/$(PKGCONF) $(PKGCONF_URL); fi
	@diff -qN mod/config build/.config || cp mod/config build/.config
	@if [ -f config.user ]; then diff -qN config.user build/.config || cp config.user build/.config; fi
	@diff -qN mod/busybox-config build/package/busybox/busybox.config || cp mod/busybox-config build/package/busybox/busybox.config
	@cp mod/0001-gmp-disable-obstack-vprintf.patch build/package/gmp
	
.patch.applied:
	tar xf gcc-4.9.3.tar
	cd build; for p in `ls ../*.patch|sort`; do patch -p1 < $$p; done
	touch $@

userconfig: .build.stamp arch-patches
	@make -C build menuconfig
	@cp build/.config config.user
	@echo new config saved to `pwd`/config.user

	
	
