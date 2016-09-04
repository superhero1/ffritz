
TOPDIR	= $(shell pwd)
VERSION = $(shell cat version)
HOST    = $(shell uname -m)
SUDO	= sudo

RELDIR  = release$(VERSION)

ARM_MODFILES = $(shell find arm/mod/ -type f -o -type d)
ATOM_MODFILES = $(shell find atom/mod/ -type f -o -type d)

# The original firmware tarball
#
ORIG=$(TOPDIR)/../original_141.06.50.tar
#ORIG=$(TOPDIR)/../FRITZ.Box_6490_Cable.de-en-es-it-fr-pl.141.06.61.image

# Keep original rootfs for diff?
# sudo dirdiff arm/orig/ arm/squashfs-root/
#
KEEP_ORIG = 1

HOSTTOOLS=$(TOPDIR)/host/$(HOST)

###############################################################################################
FWVER=$(shell if [ -f .fwver.cache ]; then cat .fwver.cache; else strings $(ORIG) | grep -i ^newFWver=|sed -e 's/.*=//' | tee .fwver.cache; fi)
###############################################################################################

ifeq ($(FWVER),)
$(error Could not determine firmware version)
else
ifneq ($(FWVER),06.50)
$(warning !!!! Firmware version $(FWVER) not tested !!!!)
endif
endif


all: arm/filesystem.image #atom/filesystem.image

###############################################################################################
## Unpack, patch and repack ARM FS
#
armfs:	arm/filesystem.image

ARM_PATCHES=$(shell cat arm/patchlist)
ARM_PATCHST=$(ARM_PATCHES:%=arm/.applied.%)

tmp/arm/filesystem.image:
	@mkdir -p tmp/arm
	@cd tmp/arm; tar xf $(ORIG) ./var/remote/var/tmp/filesystem.image --strip-components=5

arm/squashfs-root:  tmp/arm/filesystem.image 
	@if [ ! -d arm/squashfs-root ]; then cd arm; $(SUDO) $(HOSTTOOLS)/unsquashfs4-lzma-avm-be $(TOPDIR)/tmp/arm/filesystem.image; fi
	@if [ $(KEEP_ORIG) -eq 1 -a ! -d arm/orig ]; then cd arm; $(SUDO) $(HOSTTOOLS)/unsquashfs4-lzma-avm-be -d orig $(TOPDIR)/tmp/arm/filesystem.image; fi
	@$(SUDO) chmod 755 arm/squashfs-root

$(ARM_PATCHST):	$(@:arm/.applied.%=%)
	@echo APPLY $(@:arm/.applied.%=%)
	@cd arm/squashfs-root; $(SUDO) patch -p1 < $(@:arm/.applied.%=../%)
	@touch $@

arm/filesystem.image: $(ARM_MODFILES) arm/squashfs-root $(ARM_PATCHST)
	@echo "PATCH  arm/squashfs-root"
	@$(SUDO) rsync -a arm/mod/* arm/squashfs-root/
	@rm -f arm/filesystem.image
	@echo "PACK  arm/squashfs-root"
	@cd arm; $(SUDO) $(HOSTTOOLS)/mksquashfs4-lzma-avm-be squashfs-root filesystem.image -all-root -info -no-progress -no-exports -no-sparse -b 65536 >/dev/null

###############################################################################################
## Unpack, patch and repack ATOM FS (UNTESTED)
#
atomfs:	atom/filesystem.image

tmp/atom/filesystem.image:
	@mkdir -p tmp/atom
	@cd tmp/atom; tar xf $(ORIG) ./var/remote/var/tmp/x86/filesystem.image --strip-components=6

atom/squashfs-root:  tmp/atom/filesystem.image
	@if [ ! -d atom/squashfs-root ]; then cd atom; $(SUDO) unsquashfs $(TOPDIR)/tmp/atom/filesystem.image; fi
	@if [ $(KEEP_ORIG) -eq 1 -a ! -d atom/orig ]; then cd atom; $(SUDO) unsquashfs -d orig $(TOPDIR)/tmp/atom/filesystem.image; fi

atom/filesystem.image: $(ATOM_MODFILES) atom/squashfs-root
	@echo "PATCH  atom/squashfs-root"
	@$(SUDO) rsync -a atom/mod/* atom/squashfs-root/
	@rm -f atom/filesystem.image
	@echo XXX atom fs UNTESTED
	@cd atom; $(SUDO) mksquashfs squashfs-root filesystem.image -all-root -info -no-progress -no-exports -no-sparse -b 65536 >/dev/null


.PHONY:		$(RELDIR)

###############################################################################################
release:	armfs $(RELDIR) 
	@echo "PACK   $(RELDIR)/fb6490_$(FWVER)-$(VERSION).tar"
	@cp arm/filesystem.image $(RELDIR)/var/remote/var/tmp/filesystem.image
	@cd $(RELDIR); tar cf fb6490_$(FWVER)-$(VERSION).tar var
	@rm -rf $(RELDIR)/var

$(RELDIR):
	@echo "PREP   $(RELDIR)"
	@mkdir -p $(RELDIR)
	@cd $(RELDIR); ln -sf ../README.txt .
	@cd $(RELDIR); ln -sf ../INSTALL.txt .
	@cd $(RELDIR); ln -sf ../telnet-1.tar .
	@cd $(RELDIR); tar xf $(ORIG)

###############################################################################################
clean:
	rm -rf tmp
	$(SUDO) rm -rf arm/squashfs-root
	$(SUDO) rm -rf arm/orig
	rm -f arm/filesystem.image
	rm -f arm/.applied*
	$(SUDO) rm -rf atom/squashfs-root
	$(SUDO) rm -rf atom/orig
	rm -f atom/filesystem.image
	rm -f atom/.applied*
	rm -f .fwver.cache
