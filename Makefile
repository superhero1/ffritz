
TOPDIR	= $(shell pwd)
VERSION = $(shell cat version)
ARM_VER = $(shell cat packages/arm/ffritz/version)
HOST    = $(shell uname -m)
SUDO	=


################################################################################
# Configuration
################################################################################
#
# The original firmware tarball URL
#
# 6590:
#URL=http://download.avm.de/firmware/6590/96980342/FRITZ.Box_6590_Cable.de-en-es-it-fr-pl.148.07.00.image
#
# 6490:
#URL=http://download.avm.de/firmware//6490/36787213/FRITZ.Box_6490_Cable.de-en-es-it-fr-pl.141.07.00.image
#URL=http://download.avm.de/firmware//6490/70988975/FRITZ.Box_6490_Cable.de-en-es-it-fr-pl.141.07.01.image
URL=https://download.avm.de/firmware/6490/59088767/FRITZ.Box_6490_Cable.de-en-es-it-fr-pl.141.07.02.image

# where to store to/fetch from
#
ORIG=$(TOPDIR)/../$(shell basename $(URL))

# for explicit image path (e.g. Labor)
#
#ORIG=$(TOPDIR)/../FRITZ.Box_6490_Cable-07.08-67153-LabBETA.image

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

RELDIR  = release$(VERSION)

ARM_MODFILES = $(shell find arm/mod/ -type f -o -type d)
ATOM_MODFILES = $(shell find atom/mod/ -type f -o -type d)

###############################################################################################
###############################################################################################
FWVER=$(shell echo $(ORIG) | sed -e 's/.*\([0-9]*.\.[0-9]*\).*\.image/\1/')

MODEL=$(shell echo $(ORIG) | sed -e 's/.*_\(....\)_Cable.*/\1/')
#FWVER=07.00

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

all: release

$(ORIG):
	wget $(URL) -O $(ORIG)


ifneq ($(FFRITZ_ARM_PACKAGE),)
$(FFRITZ_ARM_PACKAGE):
	wget ftp://ftp.ffesh.de/pub/ffritz/arm/$(shell basename $(FFRITZ_ARM_PACKAGE)) -O $(FFRITZ_ARM_PACKAGE) || true
endif

###############################################################################################
## Unpack, patch and repack ARM FS
#
armfs:	arm/filesystem.image

ARM_PATCHES += rc.tail.patch
ARM_PATCHES += $(shell test $(FWNUM) -gt 660 && echo nvram_dontremove.patch)
ARM_PATCHES += $(shell test $(FWNUM) -lt 663 && echo ipv6_enable.patch)

ARM_PATCHST=$(ARM_PATCHES:%=arm/.applied.%)

tmp/arm/filesystem.image: $(ORIG)
	@mkdir -p tmp/arm
	@cd tmp/arm; tar xf $(ORIG) ./var/remote/var/tmp/filesystem.image --strip-components=5

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

ATOM_PATCHES = 50-udev-default.patch profile.patch rc.tail.patch

ATOM_PATCHST=$(ATOM_PATCHES:%=atom/.applied.%)

tmp/atom/filesystem.image: $(ORIG)
	@mkdir -p tmp/atom
	@cd tmp/atom; tar xf $(ORIG) ./var/remote/var/tmp/x86/filesystem.image --strip-components=6

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
	@cd atom; $(SUDO) mksquashfs squashfs-root filesystem.image -all-root -info -no-progress -no-exports -no-sparse -b 65536 -processors 1 >/dev/null

#.PHONY:		$(RELDIR)

###############################################################################################
FWFILE  = fb$(MODEL)_$(FWVER)-$(VERSION).tar

release:
	fakeroot make clean
	fakeroot make $(RELDIR)/$(FWFILE)
	
$(RELDIR)/$(FWFILE): armfs atomfs $(RELDIR) 
	@rm -rf $(RELDIR)/var
	@cd $(RELDIR); tar xf $(ORIG)
	@echo "PACK   $(RELDIR)/$(FWFILE)"
	@cp arm/filesystem.image $(RELDIR)/var/remote/var/tmp/filesystem.image
	@cp arm/mod/usr/bin/switch_bootbank $(RELDIR)/var
	@cp atom/filesystem.image $(RELDIR)/var/remote/var/tmp/x86/filesystem.image
	@cd $(RELDIR); patch -p0 < ../install.p
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
	make -C packages/x86/buildroot userconfig

rebuild:
	make -C packages base-install

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
