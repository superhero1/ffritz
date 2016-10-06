
PKGTOP	= $(shell cd ../..; pwd)
DLDIR	= $(PKGTOP)/dl

export PATH := $(TOOLCHAIN):$(PATH)

URL	= $(shell cat $(PKGTOP)/url-buildroot)
FILE	= $(DLDIR)/$(shell basename $(URL))

all:	.build.stamp
	@make -C build $(BUILDROOT_TARGETS)

$(FILE):
	@cd $(DLDIR); wget $(URL)

.build.stamp:	$(FILE)
	mkdir -p build
	cd build; tar xf $(FILE) --strip-components=1
	cp mod/config build/.config
	cp mod/alsa-lib.mk build/package/alsa-lib/alsa-lib.mk
	cp mod/uClibc-0.9.33.config build/toolchain/uClibc/uClibc-0.9.33.config
	cp mod/toolchain_gcc_Config.in build/toolchain/gcc/Config.in
	ln -sf $(DLDIR) build/dl
	touch .build.stamp

clean:
	rm -rf build .build.stamp

distclean:
	rm -rf build
	rm -f $(FILE)
	rm -f .build.stamp
