
ARM_PATCHES += $(shell test $(FWNUM) -lt 720 && echo rc.tail.patch)
ARM_PATCHES += $(shell test $(FWNUM) -gt 660 && echo nvram_dontremove.patch)
ARM_PATCHES += $(shell test $(FWNUM) -lt 663 && echo ipv6_enable.patch)

ATOM_PATCHES = $(shell test $(FWNUM) -lt 720 && echo 50-udev-default.patch)
ATOM_PATCHES += profile.patch
ATOM_PATCHES += $(shell test $(FWNUM) -lt 720 && echo rc.tail.patch)

ARM_MAX_FS=999999999

tmp/$(ARM_DIR_P6)/filesystem.image: $(ORIG)
	@mkdir -p tmp/$(ARM_DIR_P6)
	@cd tmp/$(ARM_DIR_P6); tar xf $(ORIG) ./var/remote/var/tmp/filesystem.image --strip-components=5

tmp/$(ATOM_DIR_P6)/filesystem.image: $(ORIG)
	@mkdir -p tmp/$(ATOM_DIR_P6)
	@cd tmp/$(ATOM_DIR_P6); tar xf $(ORIG) ./var/remote/var/tmp/x86/filesystem.image --strip-components=6
	
$(RELDIR)/$(FWFILE): armfs atomfs $(RELDIR) 
	@rm -rf $(RELDIR)/var
	@cd $(RELDIR); tar xf $(ORIG)
	@echo "PACK   $(RELDIR)/$(FWFILE)"
	@cp $(ARM_DIR)/filesystem.image $(RELDIR)/var/remote/var/tmp/filesystem.image
	@cp $(ATOM_DIR)/mod/usr/bin/switch_bootbank $(RELDIR)/var
	@cp $(ATOM_DIR)/filesystem.image $(RELDIR)/var/remote/var/tmp/x86/filesystem.image
	@cd $(RELDIR); patch -p0 < ../install.p
	@cd $(RELDIR); $(TAR) cf $(FWFILE) var
	@rm -rf $(RELDIR)/var
	@echo
	@echo +++ Done +++
	@echo Image is: $@
	@echo


