
PKGTOP	= $(shell cd ../..; pwd)
DLDIR	= $(PKGTOP)/dl

ifeq ($(BUILDROOT_AVM),)
URL	= $(shell cat $(PKGTOP)/url-buildroot)
else
URL	= $(shell cat $(PKGTOP)/url-buildroot-avm)
endif
FILE	= $(DLDIR)/$(shell basename $(URL))

BR_FLAGS=DL_DIR=$(DLDIR)

## For some reason i need gcc-4.7, otherwise cross gcc does not compile ..
#
## For this try to enable the following macro, and maybe replace ld with gold
#
#GCCFLAGS=HOSTCC=gcc-4.7

all:	.build.stamp arch-patches 
	make -C build $(BR_FLAGS) $(GCCFLAGS) -j2 
	if [ "$(BUILDROOT_TARGETS)" != "" ]; then make -C build $(BR_FLAGS) $(BUILDROOT_TARGETS) -j2; fi

base:	.build.stamp arch-patches
	make -C build $(BR_FLAGS) $(GCCFLAGS) -j2
	make -C build $(BR_FLAGS) $(BASE_TARGETS) -j2

$(FILE):
	@cd $(DLDIR); wget $(URL)

.build.stamp:	$(FILE) 
	mkdir -p build
	cd build; tar xf $(FILE) --strip-components=1
	touch .build.stamp

clean:
	rm -rf build .build.stamp .*.applied .toolchain.stamp

distclean:
	rm -rf build
	rm -f $(FILE)
	rm -f .build.stamp
