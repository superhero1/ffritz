BUILDROOT_TARGETS   = 
BASE_TARGETS   = openssl

include ../../buildroot.mk

arch-patches:	.patch.applied
	@if [ -f config.user ]; then install -v config.user build/.config; fi
	@if [ -f mod/busybox-config ]; then install -v mod/busybox-config build/package/busybox/busybox.config; fi
	
.patch.applied:
#	@cd build; for p in `ls ../*.patch|sort`; do patch -p1 < $$p; done
	for d in `cd package; ls -d *`; do \
	    mkdir -p build/package/$$d; \
	    install -v package/$$d/* build/package/$$d; \
	    done
	install -v mod/config build/.config
	touch $@

userconfig: .build.stamp arch-patches
	@make -C build menuconfig
	@cp build/.config config.user
	@echo new config saved to `pwd`/config.user

