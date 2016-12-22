
PKGTOP	= $(shell cd ../..; pwd)
DLDIR	= $(PKGTOP)/dl

TOPDIR	    = $(shell pwd)
BUILDROOT   = $(shell cd ../buildroot/build; pwd)
TOOLCHAIN   = $(shell cd $(BUILDROOT)/output/host/usr/bin/; pwd)
SYSROOT	    = $(BUILDROOT)/output/host/usr/i686-buildroot-linux-uclibc/sysroot

PREFIX=/usr/local

export PATH := $(TOOLCHAIN):$(PATH)
export CC   := i686-buildroot-linux-uclibc-gcc

URL	= $(shell cat $(PKGTOP)/url-lirc)
COMMIT	= $(shell cat $(PKGTOP)/commit-lirc)
FILE	= $(DLDIR)/$(shell basename $(URL))

.PHONY:	build

all:	build/config.status
	make -C build
	mkdir -p $(TOPDIR)/output
	export PATH=$(TOOLCHAIN):$(PATH); make -C build

$(FILE):
	@cd $(DLDIR); wget $(URL)

build/configure:
	git clone $(URL) build
	cd build; ./autogen.sh
	
build/config.status:	build/configure
	@cd build; export PATH=$(TOOLCHAIN):$(PATH); ./configure \
	    --host=i686-buildroot-linux-uclibc \
	    --target=i686-buildroot-linux-uclibc \
	    --localstatedir=/var \
	    --prefix=$(PREFIX)

install:
	mkdir -p $(DESTDIR)
	export PATH=$(TOOLCHAIN):$(PATH); make -C build install DESTDIR=$(TOPDIR)/output
	cp -arnv output/$(PREFIX)/bin $(DESTDIR)
	cp output/$(PREFIX)/sbin/* $(DESTDIR)/bin
	cp -arnv output/$(PREFIX)/lib $(DESTDIR)
	mkdir -p $(DESTDIR)/share
	cp -arnv output/$(PREFIX)/share/lirc $(DESTDIR)/share
	cp -arnv output/$(PREFIX)/etc $(DESTDIR)

clean:
	rm -rf build
