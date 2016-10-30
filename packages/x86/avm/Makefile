
TOPDIR	    = $(shell pwd)
DESTDIR	    = $(TOPDIR)/output

URLFILE = ../../url-avm-toolchain-atom

URL	= $(shell cat $(URLFILE))
FILE	= ../../dl/$(shell basename $(URL))

.PHONY:	build

all:	avm-prep.stamp 
	make -C build KERNEL_LAYOUT=x86 
	rm -rf GPL kernel

install:
	@make -C build PROGRAMS="dropbear dropbearkey dropbearconvert dbclient scp" STATIC=1 SCPPROGRESS=1 install DESTDIR=$(DESTDIR)
	strip $(DESTDIR)/usr/local/bin/*
	strip $(DESTDIR)/usr/local/sbin/*
	

$(FILE): $(URLFILE)
	cd ../../dl; wget $(URL)

avm-prep.stamp: $(FILE)
	tar xf $(FILE) GPL/GPL-release_kernel.tar.gz GPL/GPL-gcc.tar.gz
	mkdir -p build
	cd build; tar xf $(TOPDIR)/GPL/GPL-gcc.tar.gz
	mv build/kernel-2.6.39.3.patch build/kernel-2.6.39.3.patch_XXX_does_not_apply
	cp buildroot.config.x86 build
	mkdir -p kernel
	cd kernel; tar xf $(TOPDIR)/GPL/GPL-release_kernel.tar.gz
	cd kernel; patch -p1 < ../kernel-make.patch
	touch avm-prep.stamp

clean:
	rm -rf build
	rm -f avm-prep.stamp

distclean:
	rm -rf build
	rm -f $(FILE)
