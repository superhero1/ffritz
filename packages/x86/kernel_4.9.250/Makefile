
CONFTYPE	= legacy

NO_MAKE_INSTALL = 1
MAKE_OPTIONS    =  ARCH=x86 CROSS_COMPILE=i686-buildroot-linux-gnu- modules
CUSTOM_DEP      = .prep.stamp
AUDIO_MODULES   = sound/usb/snd-usb-audio.ko sound/usb/snd-usbmidi-lib.ko \
			sound/core/snd-rawmidi.ko sound/core/snd.ko \
			sound/core/snd-timer.ko sound/core/snd-hwdep.ko \
			sound/core/snd-pcm.ko sound/soundcore.ko

include ../../package.mk

KCONFIG=x86_avm_defconfig

.PHONY: config

.prep.stamp:
	cp $(KCONFIG) $(BUILDDIR)/arch/x86/configs/
	make -C $(BUILDDIR) ARCH=x86 CROSS_COMPILE=i686-buildroot-linux-gnu- $(KCONFIG)
	touch $@

config:
	make -C $(BUILDDIR) ARCH=x86 CROSS_COMPILE=i686-buildroot-linux-gnu- menuconfig savedefconfig

bzImage:
	PATH=$(TOOLCHAIN):$(PATH) make -C $(BUILDDIR) ARCH=x86 CROSS_COMPILE=i686-buildroot-linux-gnu- bzImage


test:
	make -C $(BUILDDIR) ARCH=x86 CROSS_COMPILE=i686-buildroot-linux-gnu- puma7-crash-kernel_defconfig

$(AUDIO_MODULES):	all

install-modules: $(AUDIO_MODULES:%=$(BUILDDIR)/%)
	mkdir -p $(DESTDIR)
	cp $? $(DESTDIR)
