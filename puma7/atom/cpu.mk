ATOM_MODFILES = $(shell find $(PLAT_BASE)/atom/mod -type f -o -type d)

ATOM_ROOTIMG = $(PLAT_TMP)/uimage/part_03_ATOM_ROOTFS.bin

ATOM_PATCHES = profile.patch

ifeq ($(shell test $(FWMAJ) -eq 7 -a $(FWMIN) -lt 19 ; echo $$?),0)
ATOM_PATCHES += 50-udev-default.patch
ATOM_PATCHES += hotplug-remap-v1.patch
else # >= 7.19
ATOM_PATCHES += 10-console.rules.patch
ATOM_PATCHES += 20-rc-net-ffmultid.patch

ifeq ($(shell test $(FWMAJ) -eq 7 -a $(FWMIN) -lt 24 ; echo $$?),0)
ATOM_PATCHES += hotplug-remap-v2.patch
else # >= 7.24
ATOM_PATCHES += hotplug-remap-v3.patch
endif

endif

ATOM_PATCHST=$(ATOM_PATCHES:%=$(ATOM_TMP)/.applied.%)

$(ATOM_ROOTIMG): $(PLAT_TMP)/uimage
