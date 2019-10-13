
TOPDIR	= $(shell pwd)
VERSION = $(shell cat version)
ARM_VER = $(shell cat packages/arm/ffritz/version)
HOST    = $(shell uname -m)
DLDIR   = $(TOPDIR)/packages/dl
SUDO	=


################################################################################
# Configuration
################################################################################
#
# The original firmware .image/.zip URL
# (or directly put the file to packages/dl)
#
# Labor image:
# https://avm.de/fileadmin/user_upload/DE/Labor/Download/fritzbox-labor_6591-71081.zip
#
# URL=https://avm.de/fileadmin/user_upload/DE/Labor/Download/fritzbox-labor_6591-71081.zip
#URL=ftp://jason:274jgjg85hh36@update.avm.de/labor/6591/labor_71700/FRITZ.Box_6591_Cable-07.08-71700-LabBETA.image
URL=ftp://jason:274jgjg85hh36@update.avm.de/labor/6591/labor_72169/FRITZ.Box_6591_Cable-07.08-72169-LabBETA.image

# Keep original rootfs for diff?
# sudo dirdiff arm/orig/ arm/squashfs-root/
#
KEEP_ORIG = 1

# The optional arm package contains some none-essential binaries for the
# arm core (not really required any more)
# DOWNLOAD to fetch binary package
#
#FFRITZ_ARM_PACKAGE=DOWNLOAD
#FFRITZ_ARM_PACKAGE=../ffritz-arm-0.7-fos7.tar.gz

## Host tools (unsquashfs4-lzma-avm-be, mksquashfs4-lzma-avm-be) can either be built
# (using squashfstools-be target), or try the pre-compiled binaries
#
#HOSTTOOLS=$(TOPDIR)/freetz/tools
HOSTTOOLS=$(TOPDIR)/host/$(HOST)

###############################################################################################

all: release

###############################################################################################
RELDIR  = release$(VERSION)

ARM_MODFILES = $(shell find arm/mod/ -type f -o -type d)
ATOM_MODFILES = $(shell find atom/mod/ -type f -o -type d)

###############################################################################################
###############################################################################################

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
BETA=$(shell echo $(ORIG) | sed -e 's/.*-\([0-9]*\)-LabBETA.*/-\1/')
MODEL=$(shell echo $(ORIG) | sed -e 's/.*_\(....\)_Cable.*/\1/')
FWNUM=$(subst .,,$(FWVER))

ifeq ($(FWVER),)
$(error Could not determine firmware version ($(ORIG) missing?))
endif

DFL_ARM_PACKAGE=packages/arm/ffritz/ffritz-arm-$(ARM_VER)-fos7.tar.gz
ifeq ($(FFRITZ_ARM_PACKAGE),DOWNLOAD)
FFRITZ_ARM_PACKAGE=$(DFL_ARM_PACKAGE)
else
FFRITZ_ARM_PACKAGE=
endif


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


ifneq ($(FFRITZ_ARM_PACKAGE),)
$(FFRITZ_ARM_PACKAGE):
	wget ftp://ftp.ffesh.de/pub/ffritz/arm/$(shell basename $(FFRITZ_ARM_PACKAGE)) -O $(FFRITZ_ARM_PACKAGE) || true
endif

src/uimg/uimg:
	@make -C src/uimg

tmp/uimage:	$(ORIG) src/uimg/uimg
	@mkdir -p tmp/uimage
	@cd tmp/uimage; tar xf $(ORIG)  
	@cd tmp/uimage; $(TOPDIR)/src/uimg/uimg -u -n part var/firmware-update.uimg

###############################################################################################
## Unpack, patch and repack ARM FS
#
armfs:	arm/filesystem.image

ARM_PATCHES += oem.patch

ARM_PATCHST=$(ARM_PATCHES:%=arm/.applied.%)

tmp/arm/filesystem.image: tmp/uimage
	@mkdir -p tmp/arm
	@cd tmp/arm; ln -sf ../uimage/part_09_ARM_ROOTFS.bin filesystem.image

arm/squashfs-root:  tmp/arm/filesystem.image 
	@if [ ! -d arm/squashfs-root ]; then cd arm; $(SUDO) $(HOSTTOOLS)/unsquashfs4-avm-be $(TOPDIR)/tmp/arm/filesystem.image; fi
	@if [ $(KEEP_ORIG) -eq 1 -a ! -d arm/orig ]; then cd arm; $(SUDO) $(HOSTTOOLS)/unsquashfs4-avm-be -d orig $(TOPDIR)/tmp/arm/filesystem.image; fi

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
	@if [ -d arm/mod/usr/local/bin ]; then  $(TOPDIR)/mklinks -f arm/squashfs-root/usr/bin ../local/bin; fi
	@touch $@

arm/filesystem.image: arm/.applied.fs
	@rm -f arm/filesystem.image
	@$(SUDO) chmod 755 arm/squashfs-root
	@echo "PACK  arm/squashfs-root"
	@cd arm; $(SUDO) $(HOSTTOOLS)/mksquashfs4-avm-be squashfs-root filesystem.image -all-root -info -no-progress -no-exports -no-sparse -b 65536 -processors 1 >/dev/null

arm-package: packages/arm/ffritz/ffritz-arm-$(ARM_VER).tar.gz

packages/arm/ffritz/ffritz-arm-$(ARM_VER).tar.gz:
	make -C packages/arm/ffritz
	@echo
	@echo Successfully built $@

###############################################################################################
## Unpack, patch and repack ATOM FS 
#
atomfs:	atom/filesystem.image

ATOM_PATCHES = 50-udev-default.patch profile.patch rc.tail.patch oem.patch

ATOM_PATCHST=$(ATOM_PATCHES:%=atom/.applied.%)

tmp/atom/filesystem.image: tmp/uimage
	@mkdir -p tmp/atom
	@cd tmp/atom; ln -sf ../uimage/part_03_ATOM_ROOTFS.bin filesystem.image

atom/squashfs-root:  tmp/atom/filesystem.image
	@if [ ! -d atom/squashfs-root ]; then cd atom; $(SUDO) unsquashfs $(TOPDIR)/tmp/atom/filesystem.image; fi
	@if [ $(KEEP_ORIG) -eq 1 -a ! -d atom/orig ]; then cd atom; $(SUDO) unsquashfs -d orig $(TOPDIR)/tmp/atom/filesystem.image; fi

$(ATOM_PATCHST):	$(@:atom/.applied.%=%)
	@echo APPLY $(@:atom/.applied.%=%)
	@cd atom/squashfs-root; $(SUDO) patch -p1 < $(@:atom/.applied.%=../%)
	@touch $@

atom/.applied.fs: $(ATOM_MODFILES) atom/squashfs-root $(ATOM_PATCHST)
	@echo "PATCH  atom/squashfs-root"
	@$(SUDO) $(RSYNC) -a atom/mod/ atom/squashfs-root/
	@mkdir -p atom/squashfs-root/usr/local
#	@packages/x86/ffritz/mkbblinks packages/x86/ffritz/bb-apps busybox-i686 atom/squashfs-root/usr/bin atom/squashfs-root/sbin atom/squashfs-root/bin atom/squashfs-root/usr/sbin atom/squashfs-root/usr/bin atom/squashfs-root/usr/local/bin
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
	fakeroot make clean
	fakeroot make $(RELDIR)/$(FWFILE)
	
$(RELDIR)/$(FWFILE): atomfs armfs $(RELDIR) 
	@rm -rf $(RELDIR)/var
	@cd $(RELDIR); tar xf $(ORIG)
	@cp -vf arm/filesystem.image tmp/uimage/*ARM_ROOTFS.bin
	@rm -f $(RELDIR)/var/remote/var/tmp/filesystem.image
	@cp atom/mod/usr/bin/switch_bootbank $(RELDIR)/var
	@cp -f atom/filesystem.image tmp/uimage/*ATOM_ROOTFS.bin
	@echo "PACK   firmware-update.uimg"
	@$(TOPDIR)/src/uimg/uimg -p -n tmp/uimage/part $(RELDIR)/var/firmware-update.uimg
	@echo "PACK   $(RELDIR)/$(FWFILE)"
	@cd $(RELDIR); $(TAR) cf $(FWFILE) var
	@rm -rf $(RELDIR)/var
	@echo
	@echo +++ Done +++
	@echo Image is: $@
	@echo

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

package-atom:	atom/squashfs-root
	make -C packages/x86

atom-brconfig:
	@make -C packages/x86/buildroot userconfig
	@echo
	@echo +++ run \"make package-atom\" to generate application image with modified configuration.
	@echo

rebuild:
	make -C packages/x86 base base-install

help:
	@echo 'Make targets:'
	@echo '------------'
	@echo 'all              : Rebuild modified file system with pre-built binaries (or those from the "rebuild" target)'
	@echo 'rebuild          : Rebuild binaries for modified filesystem images'
	@echo 'package          : Rebuild optional packages'
	@echo 'package-arm      : Rebuild optional package for arm'
	@echo 'package-atom     : Rebuild optional package for atom'
	@echo 'atom-brconfig    : Change buildroot configuration for atom'
	@echo 'squashfstools-be : Download freetz and build big endian squashfs tools for host'


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
