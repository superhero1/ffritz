
TOPDIR	    = $(shell pwd)
DESTDIR	    = $(TOPDIR)/output

URLFILE = ../../url-avm-toolchain-arm

URL	= $(shell cat $(URLFILE))
FILE	= ../../dl/$(shell basename $(URL))

# FIXME: certificate problem with curl.haxx.se, need explicit download
#
CURL	= ./build/prefix_sehr_sehr_sehr_sehr_sehr_sehr_sehr_sehr_sehr_lang_buildroot-2013.05/output/dl/curl-7.28.1.tar.bz2

.PHONY:	build

all:	avm-build.stamp
	make -C build/prefix_sehr_sehr_sehr_sehr_sehr_sehr_sehr_sehr_sehr_lang_buildroot-2013.05 readline tcpdump tcpreplay curl rsync 

avm-build.stamp:	avm-prep.stamp $(CURL)
	make -C build KERNEL_LAYOUT=armp6
	rm -rf GPL kernel
	touch avm-build.stamp

$(CURL):
	wget --no-check-certificate http://curl.haxx.se/download/curl-7.28.1.tar.bz2 -O $(CURL)

install:
	@make -C build PROGRAMS="dropbear dropbearkey dropbearconvert dbclient scp" STATIC=1 SCPPROGRESS=1 install DESTDIR=$(DESTDIR)
	strip $(DESTDIR)/usr/local/bin/*
	strip $(DESTDIR)/usr/local/sbin/*
	

$(FILE): $(URLFILE)
	cd ../../dl; wget -N $(URL) 

avm-prep.stamp: $(FILE)
	tar xf $(FILE) GPL/GPL-release_kernel.tar.gz GPL/GPL-gcc.tar.gz
	mkdir -p build
	cd build; tar xf $(TOPDIR)/GPL/GPL-gcc.tar.gz
	mv build/kernel-2.6.39.3.patch build/kernel-2.6.39.3.patch_XXX_does_not_apply
	mkdir -p kernel
	cd kernel; tar xf $(TOPDIR)/GPL/GPL-release_kernel.tar.gz
	touch avm-prep.stamp

clean:
	rm -rf build
	rm -f *.stamp

distclean:
	rm -rf build
	rm -f $(FILE)
