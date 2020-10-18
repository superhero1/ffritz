ARM_PATCHES += $(shell test $(FWNUM) -lt 720 && echo rc.tail.patch)
ARM_PATCHES += $(shell test $(FWNUM) -gt 660 && echo nvram_dontremove.patch)
ARM_PATCHES += $(shell test $(FWNUM) -lt 663 && echo ipv6_enable.patch)


ARM_ROOTIMG = $(PLAT_TMP)/var/remote/var/tmp/filesystem.image

$(ARM_ROOTIMG): $(ORIG)
	@cd $(PLAT_TMP); tar xf $(ORIG) ./var/remote/var/tmp/filesystem.image

