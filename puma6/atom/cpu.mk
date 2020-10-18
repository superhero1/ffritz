ATOM_PATCHES = $(shell test $(FWNUM) -lt 720 && echo 50-udev-default.patch)
ATOM_PATCHES += profile.patch
ATOM_PATCHES += $(shell test $(FWNUM) -lt 720 && echo rc.tail.patch)

ATOM_ROOTIMG = $(PLAT_TMP)/var/remote/var/tmp/x86/filesystem.image

$(ATOM_ROOTIMG): $(ORIG)
	@cd $(PLAT_TMP); tar xf $(ORIG) ./var/remote/var/tmp/x86/filesystem.image
