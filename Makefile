
REPODIR	= $(shell pwd)
VERSION = $(shell cat version)
REV     = $(shell git show --oneline | head -1 | sed -e 's/ .*//')
TS     := $(shell date +%D,%R)
ARM_VER = $(shell cat packages/arm/ffritz/version)
HOST    = $(shell uname -m)
DLDIR   = $(REPODIR)/dl
SUDO	= fakeroot
FWMAJ	= 0
FWMIN	= 0

all: release

config:
	@test -f conf.mk || cp -v conf.mk.dfl conf.mk
	@vi conf.mk

reconfig:
	@cp -fv conf.mk conf.mk.bak
	@cp -fv conf.mk.dfl conf.mk
	@vi conf.mk

ifeq ($(MAKECMDGOALS),clean)
NO_USER_CONF=1
endif
ifeq ($(MAKECMDGOALS),config)
NO_USER_CONF=1
endif
ifeq ($(MAKECMDGOALS),reconfig)
NO_USER_CONF=1
endif
ifeq ($(MAKECMDGOALS),rebuild)
NO_USER_CONF=1
endif

include mk/topcfg.mk

ifeq ($(HOSTTOOLS),/host/$(HOST))
HOSTTOOLS=$(REPODIR)/host/$(HOST)
endif

ifeq ($(HOSTTOOLS),)
HOSTTOOLS=$(REPODIR)/host/$(HOST)
endif

RELDIR  = $(REPODIR)/images

PLAT_BASE  := $(REPODIR)/$(SOC)
BUILD_DIR  := $(REPODIR)/build
PLAT_TMP   := $(BUILD_DIR)/$(SOC)

ARM_TMP    := $(PLAT_TMP)/arm
ARM_STAGE  := $(ARM_TMP)/squashfs-root
ARM_ORIG   := $(ARM_TMP)/orig
ARM_BASE   := $(PLAT_BASE)/arm

ATOM_TMP   := $(PLAT_TMP)/atom
ATOM_STAGE := $(ATOM_TMP)/squashfs-root
ATOM_ORIG  := $(ATOM_TMP)/orig
ATOM_BASE  := $(PLAT_BASE)/atom


ifeq ($(ITYPE),zip)
ORIG=$(DLDIR)/$(INAME)
$(ORIG):	$(DLIMAGE)
	cd $(DLDIR); unzip -oj $(DLIMAGE) $(INAME)
	touch $(ORIG)

else
# fetch update image
ORIG=$(DLDIR)/$(INAME)

$(ORIG):
	mkdir -p $(DLDIR)
	wget -O $(ORIG) $(URL)
endif

ifeq ($(FFRITZ_ARM_PACKAGE),LOCAL)
FFRITZ_ARM_PACKAGE=$(DFL_ARM_PACKAGE)
endif

BUSYBOX	:= $(shell which busybox)
RSYNC	:= $(shell which rsync)

ifeq ($(BUSYBOX),)
$(warning using tar to pack archive, recommend to install busybox)
TAR	= tar
else
TAR	= busybox tar
endif

ifeq ($(RSYNC),)
$(error rsync missing, please install)
endif

#ifneq ($(FFRITZ_ARM_PACKAGE),)
#$(FFRITZ_ARM_PACKAGE):
#	wget ftp://ftp.ffesh.de/pub/ffritz/arm/$(shell basename $(FFRITZ_ARM_PACKAGE)) -O $(FFRITZ_ARM_PACKAGE) || true
#endif

FWFILE  = fb$(MODEL)_$(FWVER)$(BETA)-$(VERSION).tar

include $(PLAT_BASE)/soc.mk

$(DFL_ARM_PACKAGE):
	@make package-arm

###############################################################################################
## Unpack, patch and repack ARM FS
#
armfs:	$(ARM_TMP)/filesystem.image

include $(ARM_BASE)/cpu.mk

ARM_PATCHES += $(shell cd $(ARM_BASE); ls user-*.patch 2>/dev/null | sort)
ARM_PATCHST=$(ARM_PATCHES:%=$(ARM_TMP)/.applied.%)

$(ARM_STAGE):  $(ARM_ROOTIMG) $(RELDIR)
	@if [ ! -d $(ARM_STAGE) ]; then cd $(ARM_TMP); $(SUDO) $(HOSTTOOLS)/unsquashfs4-avm-be $(ARM_ROOTIMG); fi
	@if [ $(KEEP_ORIG) -eq 1 -a ! -d $(ARM_ORIG) ]; then cd $(ARM_TMP); $(SUDO) $(HOSTTOOLS)/unsquashfs4-avm-be -d orig $(ARM_ROOTIMG); fi

$(ARM_PATCHST):	$(@:$(ARM_TMP)/.applied.%=$(ARM_BASE)/%)
	@echo APPLY $(@:$(ARM_TMP)/.applied.%=$(ARM_BASE)/%)
	@cd $(ARM_STAGE); $(SUDO) patch -p1 < $(@:$(ARM_TMP)/.applied.%=$(ARM_BASE)/%)
	@touch $@

$(ARM_TMP)/.applied.fs: $(ARM_MODFILES) $(ARM_STAGE) $(ARM_PATCHST) $(FFRITZ_ARM_PACKAGE)
	@echo "PATCH  $(ARM_STAGE)"
	@$(SUDO) rm -rf $(ARM_STAGE)/usr/local
	@$(SUDO) $(RSYNC) -a --no-perms $(PLAT_BASE)/arm/mod/ $(ARM_STAGE)/
	@if [ -f "$(FFRITZ_ARM_PACKAGE)" ]; then \
	    echo Integrating ARM extensions from $(FFRITZ_ARM_PACKAGE); \
	    $(SUDO) mkdir -p $(ARM_STAGE)/usr/local; \
	    $(SUDO) tar xfk $(FFRITZ_ARM_PACKAGE) --strip-components=2 -C $(ARM_STAGE)/usr/local ./ffritz-arm; \
	fi
	@touch $@

 $(ARM_TMP)/filesystem.image: $(ARM_TMP)/.applied.fs
	@rm -f $(ARM_TMP)/filesystem.image
	@$(SUDO) chmod 755 $(ARM_STAGE)
	@echo "PACK  $(ARM_STAGE)"
	@cd $(ARM_TMP); $(SUDO) $(HOSTTOOLS)/mksquashfs4-avm-be squashfs-root  $(ARM_TMP)/filesystem.image -comp xz -all-root -info -no-progress -no-exports -no-sparse -b 65536 -processors 1 >/dev/null
	@if [ `wc -c $(ARM_TMP)/filesystem.image | sed -e 's/ .*//'` -ge $(ARM_MAX_FS) ]; then \
		echo '*** ERROR: Arm filesystem is getting too large!'; \
		echo '*** Consider editing conf.mk and remove ARM extensions (FFRITZ_ARM_PACKAGE)'; \
		echo "***  size=`wc -c $(ARM_TMP)/filesystem.image | sed -e 's/ .*//'`  max=$(ARM_MAX_FS)"; \
		false; fi

arm-package: packages/arm/ffritz/ffritz-arm-$(ARM_VER).tar.gz

packages/arm/ffritz/ffritz-arm-$(ARM_VER).tar.gz:
	make -C packages/arm/ffritz
	@echo
	@echo Successfully built $@

###############################################################################################
## Unpack, patch and repack ATOM FS 
#
atomfs:	$(ATOM_TMP)/filesystem.image

include $(ATOM_BASE)/cpu.mk

ATOM_PATCHES += $(shell cd $(ATOM_BASE); ls user-*.patch 2>/dev/null | sort)
ATOM_PATCHST=$(ATOM_PATCHES:%=$(ATOM_TMP)/.applied.%)

$(ATOM_STAGE):  $(ATOM_ROOTIMG) $(RELDIR)
	@if [ ! -d $(ATOM_STAGE) ]; then cd $(ATOM_TMP); $(SUDO) unsquashfs $(ATOM_ROOTIMG); fi
	@if [ $(KEEP_ORIG) -eq 1 -a ! -d $(ATOM_ORIG) ]; then cd $(ATOM_TMP); $(SUDO) unsquashfs -d orig $(ATOM_ROOTIMG); fi

$(ATOM_PATCHST):	$(@:$(ATOM_TMP)/.applied.%=$(ATOM_BASE)/%)
	@echo APPLY $(@:$(ATOM_TMP)/.applied.%=$(ATOM_BASE)/%)
	@cd $(ATOM_STAGE); $(SUDO) patch -p1 < $(@:$(ATOM_TMP)/.applied.%=$(ATOM_BASE)/%)
	@touch $@

$(ATOM_TMP)/.applied.fs: $(ATOM_MODFILES) $(ATOM_STAGE) $(ATOM_PATCHST)
	@echo "PATCH  $(ATOM_STAGE)"
	@$(SUDO) $(RSYNC) -a $(PLAT_BASE)/atom/mod/ $(ATOM_STAGE)/
	@echo '$(VERSION) $(REV) $(TS)' > $(ATOM_STAGE)/etc/ffversion
	@mkdir -p $(ATOM_STAGE)/usr/local
	@touch $@

$(ATOM_TMP)/filesystem.image: $(ATOM_TMP)/.applied.fs
	@rm -f $(ATOM_TMP)/filesystem.image
	@$(SUDO) chmod 755 $(ATOM_STAGE)
	@echo "PACK  $(ATOM_STAGE)"
	@cd $(ATOM_TMP); $(SUDO) mksquashfs squashfs-root filesystem.image -comp xz -all-root -info -no-progress -no-sparse -b 65536 -no-xattrs -processors 1 >/dev/null

.PHONY:		$(RELDIR)

###############################################################################################

release:
	make clean
	make $(RELDIR)/$(FWFILE)
	@echo "SOC:        $(SOC)"
	@echo "MODEL:      $(MODEL)"
	@echo "FWVER:      $(FWVER)"
	@echo "LABOR:      $(BETA)"
	@echo "BR_VERSION: $(BR_VERSION)"
	@echo
	@echo Image is: $(RELDIR)/$(FWFILE)
	@echo
	
$(RELDIR):
	@echo "PREP   $(RELDIR)"
	@mkdir -p $(RELDIR)
	@mkdir -p $(ARM_TMP)
	@mkdir -p $(ATOM_TMP)

info:
	@echo "SOC:            $(SOC)"
	@echo "MODEL:          $(MODEL)"
	@echo "FWVER:          $(FWVER)"
	@echo "LABOR:          $(BETA)"
	@echo "BR_VERSION:     $(BR_VERSION)"
	@echo "Atom toolchain: $(shell make --no-print-directory -C packages/x86/buildroot$(BR_VERSION) sdk-name)"

###############################################################################################
#
# In case the packaged squashfs tools do not work ..
#

freetz:
	git clone https://github.com/Freetz/freetz.git

squashfstools-be:	freetz
	make -C freetz squashfs4-host-be
	cp freetz/tools/unsquashfs4-avm-be freetz/tools/mksquashfs4-avm-be $(HOSTTOOLS)

#
#
#
ifneq ($(SOC),puma6)
package: package-atom package-arm

package-arm:
	make -C packages/arm/ffritz

package-atom:	$(ATOM_STAGE)
	make -C packages/x86/ffritz

sdk-atom:	$(ATOM_STAGE)
	make -C packages/x86 sdk

atom-brconfig:
	@make -C packages/x86/buildroot$(BR_VERSION) userconfig
	@echo
	@echo +++ run \"make package-atom\" to generate application image with modified configuration.
	@echo

arm-brconfig:
	@make -C packages/arm/buildroot$(BR_VERSION) userconfig
	@echo
	@echo +++ run \"make package-arm\" to generate application image with modified configuration.
	@echo

rebuild:
	make -C packages/x86 base base-install BR_VERSION=-2019.05
endif

help:
	@echo 'Make targets:'
	@echo '------------'
	@echo 'all              : Rebuild modified file system with pre-built binaries (or those from the "rebuild" target)'
ifneq ($(SOC),puma6)
	@echo 'rebuild          : Rebuild binaries for modified filesystem images'
	@echo 'package          : Rebuild application packages'
	@echo 'package-arm      : Rebuild application package for arm'
	@echo 'package-atom     : Rebuild application package for atom'
	@echo 'sdk-atom         : Explicit build of sdk package for atom (if supported by selected toolchain)'
	@echo 'atom-brconfig    : Change buildroot configuration for atom'
	@echo 'arm-brconfig     : Change buildroot configuration for arm'
endif
	@echo 'squashfstools-be : Download freetz and build big endian squashfs tools for host'
	@echo 'config           : Edit configuration'
	@echo 'reconfig         : Reset and edit configuration'
	@echo 'info             : Show configuration'


###############################################################################################
clean:
	@echo Removing $(BUILD_DIR)
	@rm -rf $(BUILD_DIR)

distclean: clean
	@rm -f conf.mk
	@rm -f packages/x86/ffritz/conf.mk
