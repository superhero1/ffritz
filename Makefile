
TOPDIR	= $(shell pwd)
VERSION = $(shell cat version)
ARM_VER = $(shell cat packages/arm/ffritz/version)
HOST    = $(shell uname -m)
DLDIR   = $(TOPDIR)/packages/dl
SUDO	= fakeroot

include defaults.mk

ifeq ($(URL),)
$(error URL not set.)
endif

ifeq ($(HOSTTOOLS),)
HOSTTOOLS=$(TOPDIR)/host/$(HOST)
endif

ATOM_DIR=$(SOC)/atom
ARM_DIR=$(SOC)/arm

ATOM_DIR_P6=puma6/atom
ARM_DIR_P6=puma6/arm
ATOM_DIR_P7=puma7/atom
ARM_DIR_P7=puma7/arm

all: release

RELDIR  = release$(VERSION)

ARM_MODFILES = $(shell find $(ARM_DIR)/mod/ -type f -o -type d)
ATOM_MODFILES = $(shell find $(ATOM_DIR)/mod/ -type f -o -type d)

ITYPE=$(shell echo $(URL) | sed -e 's/.*\.//')
DLIMAGE=$(DLDIR)/$(shell basename $(URL))

ifeq ($(ITYPE),zip)
# Fetch and extract labor zip
INAME=$(shell mkdir -p $(DLDIR); if [ ! -f $(DLIMAGE) ]; then wget -O $(DLIMAGE) $(URL); fi; unzip -l $(DLIMAGE) | grep image | sed -e 's/.*\(FRITZ.*.image\)/\1/')
ifeq ($(INAME),)
error $(URL) is not a valid firmware zip
endif
ORIG=$(DLDIR)/$(INAME)

$(ORIG):	$(DLIMAGE)
	cd $(DLDIR); unzip -oj $(DLIMAGE) $(INAME)
	touch $(ORIG)

else
# fetch update image
ORIG=$(DLDIR)/$(shell basename $(URL))

$(ORIG):
	mkdir -p $(DLDIR)
	wget -O $(ORIG) $(URL)
endif

FWVER=$(shell echo $(ORIG) | sed -e 's/.*\([0-9]*.\.[0-9]*\).*\.image/\1/')
BETA=$(shell echo $(ORIG) | sed -e 's/.*-\([0-9]*\)-Lab.*/-\1/' | grep -v image)
MODEL=$(shell echo $(ORIG) | sed -e 's/.*_\(....\)_Cable.*/\1/')
FWNUM=$(subst .,,$(FWVER))

ifeq ($(FWVER),)
$(error Could not determine firmware version ($(ORIG) missing?))
endif

FWMAJ=$(shell echo $(FWVER) | sed -e 's/\..*//')
FWMIN=$(shell echo $(FWVER) | sed -e 's/.*\.//')

DFL_ARM_PACKAGE=packages/arm/ffritz/ffritz-arm-$(ARM_VER)-fos7.tar.gz

ifeq ($(FFRITZ_ARM_PACKAGE),LOCAL)
FFRITZ_ARM_PACKAGE=$(DFL_ARM_PACKAGE)
endif

FWFILE  = fb$(MODEL)_$(FWVER)$(BETA)-$(VERSION).tar

BUSYBOX	= $(shell which busybox)
RSYNC	= $(shell which rsync)

ifeq ($(BUSYBOX),)
$(warning using tar to pack archive, recommend to install busybox)
TAR	= tar
else
TAR	= busybox tar
endif

ifeq ($(RSYNC),)
$(error rsync missing, please install)
endif

include $(SOC)/soc.mk


#ifneq ($(FFRITZ_ARM_PACKAGE),)
#$(FFRITZ_ARM_PACKAGE):
#	wget ftp://ftp.ffesh.de/pub/ffritz/arm/$(shell basename $(FFRITZ_ARM_PACKAGE)) -O $(FFRITZ_ARM_PACKAGE) || true
#endif

src/uimg/uimg:
	@make -C src/uimg

tmp/uimage:	$(ORIG) src/uimg/uimg
	@mkdir -p tmp/uimage
	@cd tmp/uimage; tar xf $(ORIG)  
	@cd tmp/uimage; $(TOPDIR)/src/uimg/uimg -u -n part var/firmware-update.uimg

$(DFL_ARM_PACKAGE):
	@make package-arm

###############################################################################################
## Unpack, patch and repack ARM FS
#
armfs:	$(ARM_DIR)/filesystem.image


ARM_PATCHST=$(ARM_PATCHES:%=$(ARM_DIR)/.applied.%)


$(ARM_DIR)/squashfs-root:  tmp/$(ARM_DIR)/filesystem.image 
	@if [ ! -d $(ARM_DIR)/squashfs-root ]; then cd $(ARM_DIR); $(SUDO) $(HOSTTOOLS)/unsquashfs4-avm-be $(TOPDIR)/tmp/$(ARM_DIR)/filesystem.image; fi
	@if [ $(KEEP_ORIG) -eq 1 -a ! -d $(ARM_DIR)/orig ]; then cd $(ARM_DIR); $(SUDO) $(HOSTTOOLS)/unsquashfs4-avm-be -d orig $(TOPDIR)/tmp/$(ARM_DIR)/filesystem.image; fi

$(ARM_PATCHST):	$(@:$(ARM_DIR)/.applied.%=%)
	@echo APPLY $(@:$(ARM_DIR)/.applied.%=%)
	@cd $(ARM_DIR)/squashfs-root; $(SUDO) patch -p1 < $(@:$(ARM_DIR)/.applied.%=../%)
	@touch $@

$(ARM_DIR)/.applied.fs: $(ARM_MODFILES) $(ARM_DIR)/squashfs-root $(ARM_PATCHST) $(FFRITZ_ARM_PACKAGE)
	@echo "PATCH  $(ARM_DIR)/squashfs-root"
	@$(SUDO) rm -rf $(ARM_DIR)/squashfs-root/usr/local
	@$(SUDO) $(RSYNC) -a --no-perms $(ARM_DIR)/mod/ $(ARM_DIR)/squashfs-root/
	@if [ -f "$(FFRITZ_ARM_PACKAGE)" ]; then \
	    echo Integrating ARM extensions from $(FFRITZ_ARM_PACKAGE); \
	    $(SUDO) mkdir -p $(ARM_DIR)/squashfs-root/usr/local; \
	    $(SUDO) tar xfk $(FFRITZ_ARM_PACKAGE) --strip-components=2 -C $(ARM_DIR)/squashfs-root/usr/local ./ffritz-arm; \
	fi
# XXX
#	@if [ -d $(ARM_DIR)/squashfs-root/usr/local/bin ]; then  $(TOPDIR)/mklinks -f $(ARM_DIR)/squashfs-root/usr/bin ../local/bin; fi
	@touch $@

$(ARM_DIR)/filesystem.image: $(ARM_DIR)/.applied.fs
	@rm -f $(ARM_DIR)/filesystem.image
	@$(SUDO) chmod 755 $(ARM_DIR)/squashfs-root
	@echo "PACK  $(ARM_DIR)/squashfs-root"
	@cd $(ARM_DIR); $(SUDO) $(HOSTTOOLS)/mksquashfs4-avm-be squashfs-root filesystem.image -comp xz -all-root -info -no-progress -no-exports -no-sparse -b 65536 -processors 1 >/dev/null
	@if [ `wc -c $(ARM_DIR)/filesystem.image | sed -e 's/ .*//'` -ge $(ARM_MAX_FS) ]; then \
		echo '*** ERROR: Arm filesystem is getting too large!'; \
		echo '*** Consider editing conf.mk and remove ARM extensions (FFRITZ_ARM_PACKAGE)'; \
		echo "***  size=`wc -c $(ARM_DIR)/filesystem.image | sed -e 's/ .*//'`  max=$(ARM_MAX_FS)"; \
		false; fi

arm-package: packages/$(ARM_DIR)/ffritz/ffritz-arm-$(SOC)-$(ARM_VER).tar.gz

packages/arm/ffritz/ffritz-arm-$(SOC)-$(ARM_VER).tar.gz:
	make -C packages/$(ARM_DIR)/ffritz
	@echo
	@echo Successfully built $@

###############################################################################################
## Unpack, patch and repack ATOM FS 
#
atomfs:	$(ATOM_DIR)/filesystem.image

ATOM_PATCHST=$(ATOM_PATCHES:%=$(ATOM_DIR)/.applied.%)


$(ATOM_DIR)/squashfs-root:  tmp/$(ATOM_DIR)/filesystem.image
	@if [ ! -d $(ATOM_DIR)/squashfs-root ]; then cd $(ATOM_DIR); $(SUDO) unsquashfs $(TOPDIR)/tmp/$(ATOM_DIR)/filesystem.image; fi
	@if [ $(KEEP_ORIG) -eq 1 -a ! -d $(ATOM_DIR)/orig ]; then cd $(ATOM_DIR); $(SUDO) unsquashfs -d orig $(TOPDIR)/tmp/$(ATOM_DIR)/filesystem.image; fi

$(ATOM_PATCHST):	$(@:$(ATOM_DIR)/.applied.%=%)
	@echo APPLY $(@:$(ATOM_DIR)/.applied.%=%)
	@cd $(ATOM_DIR)/squashfs-root; $(SUDO) patch -p1 < $(@:$(ATOM_DIR)/.applied.%=../%)
	@touch $@

$(ATOM_DIR)/.applied.fs: $(ATOM_MODFILES) $(ATOM_DIR)/squashfs-root $(ATOM_PATCHST)
	@echo "PATCH  $(ATOM_DIR)/squashfs-root"
	@$(SUDO) $(RSYNC) -a $(ATOM_DIR)/mod/ $(ATOM_DIR)/squashfs-root/
	@mkdir -p $(ATOM_DIR)/squashfs-root/usr/local
	@touch $@

$(ATOM_DIR)/filesystem.image: $(ATOM_DIR)/.applied.fs
	@rm -f $(ATOM_DIR)/filesystem.image
	@$(SUDO) chmod 755 $(ATOM_DIR)/squashfs-root
	@echo "PACK  $(ATOM_DIR)/squashfs-root"
	@cd $(ATOM_DIR); $(SUDO) mksquashfs squashfs-root filesystem.image -comp xz -all-root -info -no-progress -no-sparse -b 65536 -no-xattrs -processors 1 >/dev/null

#.PHONY:		$(RELDIR)

###############################################################################################

release:
	make clean
	make $(RELDIR)/$(FWFILE)
	

$(RELDIR):
	@echo "PREP   $(RELDIR)"
	@mkdir -p $(RELDIR)

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
package: package-atom package-arm

package-arm:
	make -C packages/arm

package-atom:	$(ATOM_DIR)/squashfs-root
	make -C packages/x86

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


ifeq ($(BR_VERSION),)
rebuild:
	make -C packages/x86 base base-install
else
rebuild:
	@echo Rebuilding default binaries is not supported for BR_VERSION=$(BR_VERSION)
endif

help:
	@echo 'Make targets:'
	@echo '------------'
	@echo 'all              : Rebuild modified file system with pre-built binaries (or those from the "rebuild" target)'
	@echo 'rebuild          : Rebuild binaries for modified filesystem images'
	@echo 'package          : Rebuild optional packages'
	@echo 'package-arm      : Rebuild optional package for arm'
	@echo 'package-atom     : Rebuild optional package for atom'
	@echo 'atom-brconfig    : Change buildroot configuration for atom'
	@echo 'arm-brconfig     : Change buildroot configuration for arm'
	@echo 'squashfstools-be : Download freetz and build big endian squashfs tools for host'


###############################################################################################
clean:
	@rm -rf tmp
	@rm -rf $(ARM_DIR)/squashfs-root
	@rm -rf $(ARM_DIR)/orig
	@rm -f $(ARM_DIR)/filesystem.image
	@rm -f $(ARM_DIR)/.applied*
	@rm -rf $(ATOM_DIR)/squashfs-root
	@rm -rf $(ATOM_DIR)/orig
	@rm -f $(ATOM_DIR)/filesystem.image
	@rm -f $(ATOM_DIR)/.applied* 
	@rm -f .fwver.cache
