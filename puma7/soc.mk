
ARM_PATCHES += profile.patch $(shell cd $(ARM_DIR); ls user-*.patch 2>/dev/null | sort)

ATOM_PATCHES = profile.patch $(shell cd $(ATOM_DIR); ls user-*.patch 2>/dev/null | sort)

ifeq ($(shell test $(FWMAJ) -eq 7 -a $(FWMIN) -lt 19 ; echo $$?),0)
ATOM_PATCHES += 50-udev-default.patch
ATOM_PATCHES += hotplug-remap-v1.patch
else
ATOM_PATCHES += 10-console.rules.patch
ATOM_PATCHES += hotplug-remap-v2.patch
endif


ARM_MAX_FS=19922432

tmp/$(ARM_DIR_P7)/filesystem.image: tmp/uimage
	@mkdir -p tmp/$(ARM_DIR_P7)
	@cd tmp/$(ARM_DIR_P7); ln -sf ../../uimage/part_09_ARM_ROOTFS.bin filesystem.image

tmp/$(ATOM_DIR_P7)/filesystem.image: tmp/uimage
	@mkdir -p tmp/$(ATOM_DIR_P7)
	@cd tmp/$(ATOM_DIR_P7); ln -sf ../../uimage/part_03_ATOM_ROOTFS.bin filesystem.image

$(RELDIR)/$(FWFILE): atomfs armfs $(RELDIR) 
	@rm -rf $(RELDIR)/var
	@cd $(RELDIR); tar xf $(ORIG)
	@cp -f $(ARM_DIR)/filesystem.image tmp/uimage/*ARM_ROOTFS.bin
	@rm -f $(RELDIR)/var/remote/var/tmp/filesystem.image
	@cp $(ATOM_DIR)/mod/usr/bin/switch_bootbank $(RELDIR)/var
	@cp -f $(ATOM_DIR)/filesystem.image tmp/uimage/*ATOM_ROOTFS.bin
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
	@$(TOPDIR)/src/uimg/uimg -p -n tmp/uimage/part $(RELDIR)/var/firmware-update.uimg
	@echo "PACK   $(RELDIR)/$(FWFILE)"
	@cd $(RELDIR); $(TAR) cf $(FWFILE) var
	@rm -rf $(RELDIR)/var
	@echo
	@echo +++ Done +++
	@echo Image is: $@
	@echo

