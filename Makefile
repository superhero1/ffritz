
REPODIR	= $(shell pwd)
VERSION = $(shell cat version)
ARM_VER = $(shell cat packages/arm/ffritz/version)
HOST    = $(shell uname -m)
DLDIR   = $(REPODIR)/packages/dl
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
NOCFG=1
endif
ifeq ($(MAKECMDGOALS),config)
NOCFG=1
endif
ifeq ($(MAKECMDGOALS),reconfig)
NOCFG=1
endif

ifneq ($(NOCFG),1)
include topcfg.mk
endif


ifeq ($(HOSTTOOLS),/host/$(HOST))
HOSTTOOLS=$(REPODIR)/host/$(HOST)
endif

ifeq ($(HOSTTOOLS),)
HOSTTOOLS=$(REPODIR)/host/$(HOST)
endif

ARM_MAX_FS=19922432


RELDIR  = output

#ARM_MODFILES = $(shell find arm/mod/ -type f -o -type d)
ATOM_MODFILES = $(shell find atom/mod/ -type f -o -type d)

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

DFL_ARM_PACKAGE=packages/arm/ffritz/$(ARM_EXT_IMAGE)

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

src/uimg/uimg:
	@make -C src/uimg

tmp/uimage:	$(ORIG) src/uimg/uimg
	@mkdir -p tmp/uimage
	@cd tmp/uimage; tar xf $(ORIG)  
	@cd tmp/uimage; $(REPODIR)/src/uimg/uimg -u -n part var/firmware-update.uimg

$(DFL_ARM_PACKAGE):
	@make package-arm

###############################################################################################
## Unpack, patch and repack ARM FS
#
armfs:	arm/filesystem.image

ARM_PATCHES += profile.patch $(shell cd arm; ls user-*.patch 2>/dev/null | sort)

ARM_PATCHST=$(ARM_PATCHES:%=arm/.applied.%)

tmp/arm/filesystem.image: tmp/uimage
	@mkdir -p tmp/arm
	@cd tmp/arm; ln -sf ../uimage/part_09_ARM_ROOTFS.bin filesystem.image

arm/squashfs-root:  tmp/arm/filesystem.image 
	@if [ ! -d arm/squashfs-root ]; then cd arm; $(SUDO) $(HOSTTOOLS)/unsquashfs4-avm-be $(REPODIR)/tmp/arm/filesystem.image; fi
	@if [ $(KEEP_ORIG) -eq 1 -a ! -d arm/orig ]; then cd arm; $(SUDO) $(HOSTTOOLS)/unsquashfs4-avm-be -d orig $(REPODIR)/tmp/arm/filesystem.image; fi

$(ARM_PATCHST):	$(@:arm/.applied.%=%)
	@echo APPLY $(@:arm/.applied.%=%)
	@cd arm/squashfs-root; $(SUDO) patch -p1 < $(@:arm/.applied.%=../%)
	@touch $@

arm/.applied.fs: $(ARM_MODFILES) arm/squashfs-root $(ARM_PATCHST) $(FFRITZ_ARM_PACKAGE)
	@echo "PATCH  arm/squashfs-root"
	@$(SUDO) rm -rf arm/squashfs-root/usr/local
	@$(SUDO) $(RSYNC) -a --no-perms arm/mod/ arm/squashfs-root/
	@if [ -f "$(FFRITZ_ARM_PACKAGE)" ]; then \
	    echo Integrating ARM extensions from $(FFRITZ_ARM_PACKAGE); \
	    $(SUDO) mkdir -p arm/squashfs-root/usr/local; \
	    $(SUDO) tar xfk $(FFRITZ_ARM_PACKAGE) --strip-components=2 -C arm/squashfs-root/usr/local ./ffritz-arm; \
	fi
#	@if [ -d arm/squashfs-root/usr/local/bin ]; then  $(REPODIR)/mklinks -f arm/squashfs-root/usr/bin ../local/bin; fi
	@touch $@

arm/filesystem.image: arm/.applied.fs
	@rm -f arm/filesystem.image
	@$(SUDO) chmod 755 arm/squashfs-root
	@echo "PACK  arm/squashfs-root"
	@cd arm; $(SUDO) $(HOSTTOOLS)/mksquashfs4-avm-be squashfs-root filesystem.image -comp xz -all-root -info -no-progress -no-exports -no-sparse -b 65536 -processors 1 >/dev/null
	@if [ `wc -c arm/filesystem.image | sed -e 's/ .*//'` -ge $(ARM_MAX_FS) ]; then \
		echo '*** ERROR: Arm filesystem is getting too large!'; \
		echo '*** Consider editing conf.mk and remove ARM extensions (FFRITZ_ARM_PACKAGE)'; \
		echo "***  size=`wc -c arm/filesystem.image | sed -e 's/ .*//'`  max=$(ARM_MAX_FS)"; \
		false; fi

arm-package: packages/arm/ffritz/ffritz-arm-$(ARM_VER).tar.gz

packages/arm/ffritz/ffritz-arm-$(ARM_VER).tar.gz:
	make -C packages/arm/ffritz
	@echo
	@echo Successfully built $@

###############################################################################################
## Unpack, patch and repack ATOM FS 
#
atomfs:	atom/filesystem.image

ATOM_PATCHES = profile.patch $(shell cd atom; ls user-*.patch 2>/dev/null | sort)

ifeq ($(shell test $(FWMAJ) -eq 7 -a $(FWMIN) -lt 19 ; echo $$?),0)
ATOM_PATCHES += 50-udev-default.patch
ATOM_PATCHES += hotplug-remap-v1.patch
else
ATOM_PATCHES += 10-console.rules.patch
ATOM_PATCHES += hotplug-remap-v2.patch
endif

ATOM_PATCHST=$(ATOM_PATCHES:%=atom/.applied.%)

tmp/atom/filesystem.image: tmp/uimage
	@mkdir -p tmp/atom
	@cd tmp/atom; ln -sf ../uimage/part_03_ATOM_ROOTFS.bin filesystem.image

atom/squashfs-root:  tmp/atom/filesystem.image
	@if [ ! -d atom/squashfs-root ]; then cd atom; $(SUDO) unsquashfs $(REPODIR)/tmp/atom/filesystem.image; fi
	@if [ $(KEEP_ORIG) -eq 1 -a ! -d atom/orig ]; then cd atom; $(SUDO) unsquashfs -d orig $(REPODIR)/tmp/atom/filesystem.image; fi

$(ATOM_PATCHST):	$(@:atom/.applied.%=%)
	@echo APPLY $(@:atom/.applied.%=%)
	@cd atom/squashfs-root; $(SUDO) patch -p1 < $(@:atom/.applied.%=../%)
	@touch $@

atom/.applied.fs: $(ATOM_MODFILES) atom/squashfs-root $(ATOM_PATCHST)
	@echo "PATCH  atom/squashfs-root"
	@$(SUDO) $(RSYNC) -a atom/mod/ atom/squashfs-root/
	@mkdir -p atom/squashfs-root/usr/local
	@touch $@

atom/filesystem.image: atom/.applied.fs
	@rm -f atom/filesystem.image
	@$(SUDO) chmod 755 atom/squashfs-root
	@echo "PACK  atom/squashfs-root"
	@cd atom; $(SUDO) mksquashfs squashfs-root filesystem.image -comp xz -all-root -info -no-progress -no-sparse -b 65536 -no-xattrs -processors 1 >/dev/null

#.PHONY:		$(RELDIR)

###############################################################################################
FWFILE  = fb$(MODEL)_$(FWVER)$(BETA)-$(VERSION).tar

release:
	make clean
	make $(RELDIR)/$(FWFILE)
	
$(RELDIR)/$(FWFILE): atomfs armfs $(RELDIR) 
	@rm -rf $(RELDIR)/var
	@cd $(RELDIR); tar xf $(ORIG)
	@cp -f arm/filesystem.image tmp/uimage/*ARM_ROOTFS.bin
	@rm -f $(RELDIR)/var/remote/var/tmp/filesystem.image
	@cp atom/mod/usr/bin/switch_bootbank $(RELDIR)/var
	@cp -f atom/filesystem.image tmp/uimage/*ATOM_ROOTFS.bin
ifeq ($(ENABLE_CONSOLE),1)
	@echo "PATCH  part_02_ATOM_KERNEL.bin"
	@mkdir -p tmp/mnt
	@sudo mount -o loop tmp/uimage/part_02_ATOM_KERNEL.bin tmp/mnt >/dev/null 
	@test -r tmp/mnt/EFI/BOOT/startup.nsh
	@(echo mm 0xfed94810 0x00914b49 -w 4; echo mm 0xfed94820 0x00914b49 -w 4; cat tmp/mnt/EFI/BOOT/startup.nsh) > .startup.nsh
	@sudo cp .startup.nsh tmp/mnt/EFI/BOOT/startup.nsh
	@sudo umount tmp/mnt >/dev/null 
	@rm -f .startup.nsh
endif
	@echo "PACK   firmware-update.uimg"
	@$(REPODIR)/src/uimg/uimg -p -n tmp/uimage/part $(RELDIR)/var/firmware-update.uimg
	@echo "PACK   $(RELDIR)/$(FWFILE)"
	@cd $(RELDIR); $(TAR) cf $(FWFILE) var
	@rm -rf $(RELDIR)/var
	@echo
	@echo +++ Done +++
	@echo "SOC:        $(SOC)"
	@echo "MODEL:      $(MODEL)"
	@echo "FWVER:      $(FWVER)"
	@echo "LABOR:      $(BETA)"
	@echo "BR_VERSION: $(BR_VERSION)"
	@echo
	@echo Image is: $@
	@echo

$(RELDIR):
	@echo "PREP   $(RELDIR)"
	@mkdir -p $(RELDIR)

info:
	@echo "SOC:        $(SOC)"
	@echo "MODEL:      $(MODEL)"
	@echo "FWVER:      $(FWVER)"
	@echo "LABOR:      $(BETA)"
	@echo "BR_VERSION: $(BR_VERSION)"

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

package-atom:	atom/squashfs-root
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
	@echo 'config           : Edit configuration'
	@echo 'reconfig         : Reset and edit configuration'


###############################################################################################
clean:
	@rm -rf tmp
	@$(SUDO) rm -rf arm/squashfs-root
	@$(SUDO) rm -rf arm/orig
	@rm -f arm/filesystem.image
	@rm -f arm/.applied*
	@$(SUDO) rm -rf atom/squashfs-root
	@$(SUDO) rm -rf atom/orig
	@rm -f atom/filesystem.image
	@rm -f atom/.applied* 
	@rm -f .fwver.cache

distclean: clean
	@rm -f conf.mk
	@rm -f packages/x86/ffritz/conf.mk
