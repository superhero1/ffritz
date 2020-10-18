
ARCHDIR=$(shell while test -f arch.mk && echo $$PWD && exit 0; test $$PWD != /; do cd ..; done)

ifneq ($(BR_VERSION_OVERRIDE),)
BR_VERSION = $(BR_VERSION_OVERRIDE)
endif

ifeq ($(PATCHES),)
PATCHES=$(wildcard *.patch)
endif

ifeq ($(BASE_TARGETS),)
BASE_TARGETS   = openssl libtirpc socat
endif

ifeq ($(BUILDROOT_AVM),)
UNAME   = buildroot$(BR_VERSION)
else
UNAME   = buildroot-avm
endif


URL	= $(call URLGET,$(UNAME))
SHA	= $(call SHAGET,$(UNAME))

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

# If there is a sdk- directory corresponding to this one
# assume it can build the SDK for this buildroot
#
SDK_BUILDDIR := $(wildcard $(ARCHDIR)/sdk$(BR_VERSION))
ifneq ($(SDK_BUILDDIR),)
SDK_DST      := $(DLDIR)/toolchain-external-custom
SDK_VER      := $(shell cat $(SDK_BUILDDIR)/sdk-version)
SDK_NAME     := sdk$(BR_VERSION)-$(HOST)
SDK_FILE     := $(SDK_NAME)-$(SDK_VER).tar.gz
SDK          := $(SDK_DST)/$(SDK_FILE)
endif

# Check if external SDK image is required
#
ifneq ($(shell grep BR2_TOOLCHAIN_EXTERNAL=y $(DEFCONF_FILE)),)
SDK_DEP      := $(SDK)
endif

all:	$(SDK_DEP) $(BUILDDIR) $(DEFCONF_TGT)
ifeq ($(DEFCONF),user_defconfig)
	@touch mod/ffritz_defconfig
endif
	make -C build $(BR_FLAGS) $(BR_MAIN_TGT)
	@if [ "$(BUILDROOT_TARGETS)" != "" ]; \
		then make -C build $(BR_FLAGS) $(BUILDROOT_TARGETS); fi

toolchain: $(BUILDDIR) $(DEFCONF_TGT)
	@make -C build $(BR_FLAGS) toolchain

$(DEFCONF_TGT): $(DEFCONF_FILE)
	cp $(DEFCONF_FILE) $(DEFCONF_TGT)
ifneq ($(PARALLEL),)
	@echo 'BR2_PER_PACKAGE_DIRECTORIES=y' >> $(DEFCONF_TGT)
else
	@sed -ie 's@BR2_PER_PACKAGE_DIRECTORIES=y@BR2_PER_PACKAGE_DIRECTORIES=n@' $(DEFCONF_TGT)
endif
	sed -ie "s@^\(BR2_TOOLCHAIN_EXTERNAL_URL=\).*@\1\""$(SDK)\""@" $(DEFCONF_TGT) || true
	@echo Using configuration: $(DEFCONF)
	@make -C $(BUILDDIR) $(DEFCONF)

conf:	build $(DEFCONF_TGT)

base:	$(SDK) $(BUILDDIR) $(DEFCONF_TGT) $(PACKAGE_MOD_TGT)
	make -C build $(BR_FLAGS) toolchain
ifneq ($(BASE_TARGETS),)
	make -C build $(BR_FLAGS) $(BASE_TARGETS)
endif

userconfig: $(BUILDDIR) $(PACKAGE_MOD_TGT)
	@if [ ! -f build/configs/user_defconfig ]; then cp build/configs/ffritz_defconfig build/configs/user_defconfig; fi
	@make -C build user_defconfig 
	@make -C build menuconfig
	@make -C build savedefconfig
	@cp build/configs/user_defconfig user_defconfig
	@echo
	@echo +++ new config saved to `pwd`/user_defconfig
	@echo 

$(FILE):
	$(call WGET,$(URL),$(FILE),$(SHA_PREFIX)$(UNAME))

$(BUILDDIR): $(FILE) $(PATCHES)
	@mkdir -p $(BUILDDIR)
	@cd $(BUILDDIR); tar xf $(FILE) --strip-components=1
	@cd build; for p in $(PATCHES); do \
		patch -p1 < ../$$p; \
	done
	@cp -ar `readlink -f package` $(BUILDDIR)
	@touch $(BUILDDIR)

clean:
	rm -rf build .*.stamp .*.applied

ifneq ($(SDK_DEP),)
#
# Build in a directory depending on a SDK (buildroot- directory)
#
ifeq ($(SDK_DOWNLOAD),1)
#
# Download SDK image
#
$(SDK):	$(URL_PREFIX)$(SDK_NAME)
	@mkdir -p $(SDK_DST)
	@$(call WGET_EXT2,$(SDK_NAME),$@)
else
#
# Rebuild SDK image
#
$(SDK):
	$(SUBMAKE) $(SDK_BUILDDIR) sdk
endif
else # SDK_DEP
#
# In SDK workspace: make sdk
#
$(SDK):	$(BUILDDIR) $(DEFCONF_TGT)
	@make -C build $(BR_FLAGS) sdk
	@mkdir -p `dirname $@`
	@mv build/output/images/*sdk* $@

endif

sdk:	$(SDK)
	@echo
	@echo +++ SDK image is: $(SDK)
	@echo

sdk-clean:
	rm -f $(SDK)

sdk-name:
	@test -z $(SDK) && echo internal || echo $(SDK)

sdk-build:
ifeq ($(SDK_BUILDDIR),)
	@echo buildroot$(BR_VERSION) is not configured for external SDK
else
	@$(SUBMAKE) $(SDK_BUILDDIR) sdk SDK_DOWNLOAD=0
endif
