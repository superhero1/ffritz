
ARCHDIR=$(shell while test -f arch.mk && echo $$PWD && exit 0; test $$PWD != /; do cd ..; done)
include $(ARCHDIR)/../paths.mk

ifneq ($(BR_VERSION_OVERRIDE),)
BR_VERSION = $(BR_VERSION_OVERRIDE)
endif

ifeq ($(PATCHES),)
PATCHES=$(wildcard *.patch)
endif

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

DEFCONF_FILE = $(wildcard user_defconfig)
ifeq ($(DEFCONF_FILE),)
DEFCONF_FILE = mod/ffritz_defconfig
endif
DEFCONF = $(shell basename $(DEFCONF_FILE))
DEFCONF_TGT = $(BUILDDIR)/configs/$(DEFCONF)

PACKAGE_MODS = $(shell find package | grep -v '^package$$')
PACKAGE_MOD_TGT = $(PACKAGE_MODS:%=build/%)

all:	$(BUILDDIR) $(PACKAGE_MOD_TGT) $(DEFCONF_TGT)
ifeq ($(DEFCONF),user_defconfig)
	@touch mod/ffritz_defconfig
endif
	@if [ -r .clean ]; then \
		for d in `sort -u .clean`; do \
			echo Re-generating build/package/$$d ..;\
			test -z $$d || rm -rf build/package/$$d; \
			tar xf $(FILE) --strip-components=1 -C $(BUILDDIR) --wildcards ''\*/package/$$d'';\
			cp -r package/$$d build/package; \
		done;\
		make -C build `sort -u .clean | sed -e 's/$$/-dirclean/'`; \
	fi
	@rm -f .clean
	@make -C build $(BR_FLAGS)
	@if [ "$(BUILDROOT_TARGETS)" != "" ]; \
		then make -C build $(BR_FLAGS) $(BUILDROOT_TARGETS); fi

$(DEFCONF_TGT): $(DEFCONF_FILE)
	@cp $(DEFCONF_FILE) $(DEFCONF_TGT)
ifneq ($(PARALLEL),)
	@echo 'BR2_PER_PACKAGE_DIRECTORIES=y' >> $(DEFCONF_TGT)
else
	@sed -ie 's/BR2_PER_PACKAGE_DIRECTORIES=y/BR2_PER_PACKAGE_DIRECTORIES=n/' $(DEFCONF_TGT)
endif
	echo Using configuration: $(DEFCONF)
	@make -C $(BUILDDIR) $(DEFCONF)

$(PACKAGE_MOD_TGT): $(PACKAGE_MODS)
	@if [ $(@:build/%=%) -nt $@ ]; then \
		echo $@ | sed -e 's@build/package/\([^/]*\).*@\1@' >> .clean;\
	fi

base:	$(BUILDDIR) $(DEFCONF_TGT) $(PACKAGE_MOD_TGT)
	make -C build $(BR_FLAGS) $(TOOLCHAIN)
ifneq ($(BASE_TARGETS),)
	make -C build $(BR_FLAGS) $(BASE_TARGETS)
endif

	touch $@

userconfig: $(BUILDDIR) $(PACKAGE_MOD_TGT)
	@if [ ! -f build/configs/user_defconfig ]; then cp build/configs/ffritz_defconfig build/configs/user_defconfig; fi
	@make -C build user_defconfig 
	@make -C build menuconfig
	@make -C build savedefconfig
	@cp build/configs/user_defconfig user_defconfig;
	@echo
	@echo +++ new config saved to `pwd`/user_defconfig
	@echo 

$(FILE):
	cd $(DLDIR); wget $(URL)

$(BUILDDIR): $(FILE) $(PATCHES)
	@mkdir -p $(BUILDDIR)
	@cd $(BUILDDIR); tar xf $(FILE) --strip-components=1
	@cd build; for p in $(PATCHES); do \
		patch -p1 < ../$$p |\
		grep 'patching file package/' |\
		sed -e 's@.*package/\([^/]*\).*@\1@' >> ../.clean;\
	done
	@touch $(BUILDDIR)

clean:
	rm -rf build .clean .*.stamp .*.applied sdk-buildroot

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
