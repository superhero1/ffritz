ARM_MAX_FS=19922432

DFL_ARM_PACKAGE=packages/arm/ffritz/$(ARM_EXT_IMAGE)

ARM_ROOTIMG = $(PLAT_TMP)/uimage/part_09_ARM_ROOTFS.bin

$(ARM_ROOTIMG): $(PLAT_TMP)/uimage
