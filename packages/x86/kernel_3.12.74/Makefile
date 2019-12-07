
CONFTYPE	= legacy

NO_MAKE_INSTALL = 1
MAKE_OPTIONS    =  ARCH=x86 CROSS_COMPILE=i686-buildroot-linux-gnu- modules
CUSTOM_DEP      = .prep.stamp

all:

.prep.stamp:
	cp x86_avm_defconfig build/arch/x86/configs/
	make -C build ARCH=x86 CROSS_COMPILE=i686-buildroot-linux-gnu- x86_avm_defconfig
	touch $@

config:
	make -C build ARCH=x86 CROSS_COMPILE=i686-buildroot-linux-gnu- menuconfig savedefconfig

bzImage:
	make -C build ARCH=x86 CROSS_COMPILE=i686-buildroot-linux-gnu- bzImage

include ../../package.mk

test:
	make -C build ARCH=x86 CROSS_COMPILE=i686-buildroot-linux-gnu- puma7-crash-kernel_defconfig
