
ARCHDIR=$(shell while test -f arch.mk && echo $$PWD && exit 0; test $$PWD != /; do cd ..; done)
include $(ARCHDIR)/../paths.mk

ifneq ($(BR_VERSION_OVERRIDE),)
BR_VERSION = $(BR_VERSION_OVERRIDE)
endif

ifeq ($(PATCHES),)
PATCHES=$(wildcard *.patch)
endif
PATCHST=$(PATCHES:%=.%.applied)

ifeq ($(BUILDROOT_AVM),)
URL	= $(shell cat $(PKGTOP)/url-buildroot$(BR_VERSION))
else
URL	= $(shell cat $(PKGTOP)/url-buildroot-avm)
endif

FILE	= $(DLDIR)/$(shell basename $(URL))

ifeq ($(BR2_DL_DIR),)
BR_FLAGS=BR2_DL_DIR=$(DLDIR)
endif

BUILDDIR=build

ifneq ($(PARALLEL),)
BR_FLAGS += -j$(PARALLEL)
endif

all:	.build.stamp patches .config_all.stamp
	make -C build $(BR_FLAGS) $(GCCFLAGS)
	if [ "$(BUILDROOT_TARGETS)" != "" ]; then make -C build $(BR_FLAGS) $(BUILDROOT_TARGETS) -j2; fi

base:	.build.stamp patches
	make -C build $(BR_FLAGS) $(GCCFLAGS) -j2
	make -C build $(BR_FLAGS) $(BASE_TARGETS) -j2

patches: $(PATCHST) .pkg.applied

.pkg.applied:
	for d in `cd package; ls -d *`; do \
	    mkdir -p build/package/$$d; \
	    cp -ar --backup package/$$d/* build/package/$$d; \
	    done
	cp mod/*_defconfig build/configs
ifneq ($(PARALLEL),)
	echo 'BR2_PER_PACKAGE_DIRECTORIES=y' >> build/configs/ffritz_defconfig
	echo 'BR2_PER_PACKAGE_DIRECTORIES=y' >> build/configs/ffritz_sdk_defconfig
else
	sed -ie 's/BR2_PER_PACKAGE_DIRECTORIES=y/BR2_PER_PACKAGE_DIRECTORIES=n/' build/configs/ffritz_defconfig
	sed -ie 's/BR2_PER_PACKAGE_DIRECTORIES=y/BR2_PER_PACKAGE_DIRECTORIES=n/' build/configs/ffritz_sdk_defconfig
endif
	touch $@

.config_all.stamp:
	rm -f .config_*.stamp
	@if [ -f user_defconfig ]; then install -v user_defconfig build/configs/user_defconfig; make -C build user_defconfig; else make -C build ffritz_defconfig; fi
	touch $@

$(PATCHST): $(@:.%.applied=%)
	@echo Applying $(@:.%.applied=%)
	@cd build; patch -p1 < ../$(@:.%.applied=%)
	@touch $@

userconfig: .build.stamp
	@if [ ! -f build/configs/user_defconfig ]; then cp build/configs/ffritz_defconfig build/configs/user_defconfig; fi
	@make -C build user_defconfig 
	@make -C build menuconfig
	@make -C build savedefconfig
	@cp build/configs/user_defconfig user_defconfig;
	@echo
	@echo +++ new config saved to `pwd`/user_defconfig
	@echo 

$(FILE):
	@cd $(DLDIR); wget $(URL)

.build.stamp:	$(FILE) 
	mkdir -p build
	cd build; tar xf $(FILE) --strip-components=1
	touch .build.stamp

clean:
	rm -rf build .*.stamp .*.applied sdk-buildroot

sdk:	sdk-buildroot/relocate-sdk.sh

sdk-buildroot/relocate-sdk.sh: $(HOST)_sdk-buildroot.tar.gz
	rm -rf sdk-buildroot
	tar xfm $<
	mv $(HOST)_sdk-buildroot sdk-buildroot
	cd sdk-buildroot; ./relocate-sdk.sh 

$(HOST)_sdk-buildroot.tar.gz:
	make .build.stamp patches
	rm -f .config_*.stamp
	make -C build ffritz_sdk_defconfig
	touch .config_sdk.stamp
	make -C build sdk $(BR_FLAGS) $(GCCFLAGS)
	mv build/output/images/$(HOST)_sdk-buildroot.tar.gz $@
	touch $@
