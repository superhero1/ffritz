
PKGTOP	= $(shell cd ../..; pwd)
DLDIR	= $(PKGTOP)/dl

URL	= $(shell cat $(PKGTOP)/url-buildroot)
FILE	= $(DLDIR)/$(shell basename $(URL))

## For some reason i need gcc-4.7, otherwise cross gcc does not compile ..
#
## For this try to enable the following macro, and maybe replace ld with gold
#
#GCCFLAGS=HOSTCC=gcc-4.7

all:	.build.stamp arch-patches 
	@make -C build $(GCCFLAGS) -j2 
	@if [ "$(BUILDROOT_TARGETS)" != "" ]; then make -C build $(BUILDROOT_TARGETS) -j2; fi

base:	.build.stamp arch-patches
	@make -C build $(GCCFLAGS) -j2
	@make -C build $(BASE_TARGETS) -j2

$(FILE):
	@cd $(DLDIR); wget $(URL)

.build.stamp:	$(FILE) 
	mkdir -p build
	mkdir -p $(DLDIR)
	cd build; tar xf $(FILE) --strip-components=1
	ln -sf $(DLDIR) build/dl
	touch .build.stamp

clean:
	rm -rf build .build.stamp .*.applied .toolchain.stamp

distclean:
	rm -rf build
	rm -f $(FILE)
	rm -f .build.stamp
