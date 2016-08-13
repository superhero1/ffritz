
TOPDIR	= $(shell pwd)
VERSION = $(shell cat version)

RELDIR  = release$(VERSION)

ARM_MODFILES = $(shell find arm/mod/ -type f -o -type d)
ATOM_MODFILES = $(shell find atom/mod/ -type f -o -type d)

# The original firmware tarball
#
ORIG=$(TOPDIR)/../original_141.06.50.tar

# Keep original rootfs for diff?
# sudo dirdiff arm/orig/ arm/squashfs-root/
#
KEEP_ORIG = 1

###############################################################################################
###############################################################################################

all: arm/filesystem.image atom/filesystem.image

###############################################################################################
## Unpack, patch and repack ARM FS
#
armfs:	arm/filesystem.image

tmp/arm/filesystem.image:
	@mkdir -p tmp/arm
	@cd tmp/arm; tar xf $(ORIG) ./var/remote/var/tmp/filesystem.image --strip-components=5

arm/squashfs-root:  tmp/arm/filesystem.image
	@if [ ! -d arm/squashfs-root ]; then cd arm; sudo $(TOPDIR)/unsquashfs4_avm_x86 $(TOPDIR)/tmp/arm/filesystem.image; fi
	@if [ $(KEEP_ORIG) -eq 1 -a ! -d arm/orig ]; then cd arm; sudo $(TOPDIR)/unsquashfs4_avm_x86 -d orig $(TOPDIR)/tmp/arm/filesystem.image; fi

arm/filesystem.image: $(ARM_MODFILES) arm/squashfs-root
	@echo "PATCH  arm/squashfs-root"
	@sudo rsync -arv arm/mod/* arm/squashfs-root/
	@rm -f arm/filesystem.image
	@cd arm; sudo $(TOPDIR)/mksquashfs4-lzma-avm-be squashfs-root filesystem.image -all-root -info -no-progress -no-exports -no-sparse -b 65536;


###############################################################################################
## Unpack, patch and repack ATOM FS (UNTESTED)
#
atomfs:	atom/filesystem.image

tmp/atom/filesystem.image:
	@mkdir -p tmp/atom
	@cd tmp/atom; tar xf $(ORIG) ./var/remote/var/tmp/x86/filesystem.image --strip-components=6

atom/squashfs-root:  tmp/atom/filesystem.image
	@if [ ! -d atom/squashfs-root ]; then cd atom; sudo unsquashfs $(TOPDIR)/tmp/atom/filesystem.image; fi
	@if [ $(KEEP_ORIG) -eq 1 -a ! -d atom/orig ]; then cd atom; sudo unsquashfs -d orig $(TOPDIR)/tmp/atom/filesystem.image; fi

atom/filesystem.image: $(ATOM_MODFILES) atom/squashfs-root
	@echo "PATCH  atom/squashfs-root"
	@sudo rsync -arv atom/mod/* atom/squashfs-root/
	@rm -f atom/filesystem.image
	@echo XXX atom fs UNTESTED
	@cd atom; sudo mksquashfs squashfs-root filesystem.image -all-root -info -no-progress -no-exports -no-sparse -b 65536;


.PHONY:		$(RELDIR)

###############################################################################################
release:	armfs $(RELDIR) 
	@echo "PACK   $(RELDIR)/fb6490_6.50_telnet-$(VERSION).tar"
	@cp arm/filesystem.image $(RELDIR)/var/remote/var/tmp/filesystem.image
	@cd $(RELDIR); tar cf fb6490_6.50_telnet-$(VERSION).tar var
	@rm -rf $(RELDIR)/var

$(RELDIR):
	@echo "PREP   $(RELDIR)"
	mkdir -p $(RELDIR)
	cd $(RELDIR); ln -sf ../mksquashfs4-lzma-avm-be .
	cd $(RELDIR); ln -sf ../unsquashfs4_avm_x86 .
	cd $(RELDIR); ln -sf ../README.txt .
	cd $(RELDIR); ln -sf ../telnet-1.tar .
	cd $(RELDIR); tar xf $(ORIG)

###############################################################################################
clean:
	rm -rf tmp
	sudo rm -rf arm/squashfs-root
	rm -f arm/filesystem.image
	sudo rm -rf atom/squashfs-root
	rm -f atom/filesystem.image
