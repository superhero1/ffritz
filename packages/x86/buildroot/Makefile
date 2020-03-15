BUILDROOT_TARGETS   = 
BASE_TARGETS   = openssl

include ../../buildroot.mk

arch-patches:	.patch.applied
	@if [ -f user_defconfig ]; then install -v user_defconfig build/configs/user_defconfig; make -C build user_defconfig; fi
	
.patch.applied:
	for d in `cd package; ls -d *`; do \
	    mkdir -p build/package/$$d; \
	    cp -ar --backup package/$$d/* build/package/$$d; \
#	    install -v package/$$d/* build/package/$$d; \
	    done
	cp mod/ffritz_atom_defconfig build/configs
	make -C build ffritz_atom_defconfig
	touch $@

userconfig: .build.stamp arch-patches
	@if [ ! -f build/configs/user_defconfig ]; then cp build/configs/ffritz_atom_defconfig build/configs/user_defconfig; fi
	@make -C build user_defconfig 
	@make -C build menuconfig
	@make -C build savedefconfig
	@cp build/configs/user_defconfig user_defconfig;
	@echo
	@echo +++ new config saved to `pwd`/user_defconfig
	@echo 

