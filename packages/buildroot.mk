
PKGTOP	= $(shell cd ../..; pwd)
DLDIR	= $(PKGTOP)/dl

export PATH := $(TOOLCHAIN):$(PATH)

URL	= $(shell cat $(PKGTOP)/url-buildroot)
FILE	= $(DLDIR)/$(shell basename $(URL))

all:	.build.stamp arch-patches
	@make -C build toolchain CC=gcc-4.7 HOSTCC=gcc-4.7
	@make -C build $(BUILDROOT_TARGETS)

$(FILE):
	@cd $(DLDIR); wget $(URL)

.build.stamp:	$(FILE) 
	mkdir -p build
	mkdir -p $(DLDIR)
	cd build; tar xf $(FILE) --strip-components=1
	ln -sf $(DLDIR) build/dl
	touch .build.stamp

clean:
	rm -rf build .build.stamp

distclean:
	rm -rf build
	rm -f $(FILE)
	rm -f .build.stamp
