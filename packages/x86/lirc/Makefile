
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
	mkdir -p $(TOPDIR)/output
	make -k -C build/doc; true
	make -C build

$(FILE):
	@cd $(DLDIR); wget $(URL)

build/configure:
	git clone $(URL) build
	cd build; git reset --hard $(COMMIT)
	cd build; ./autogen.sh
	
build/config.status:	build/configure
	@cd build; export PATH=$(TOOLCHAIN):$(PATH); ./configure \
	    --host=i686-buildroot-linux-uclibc \
	    --target=i686-buildroot-linux-uclibc \
	    --localstatedir=/var \
	    --prefix=$(PREFIX)

BFILE = build/daemons/lircd
DFILE1 = $(TOPDIR)/output/$(PREFIX)/sbin/lircd
DFILE2 = $(DESTDIR)/bin/lircd

install: $(DFILE2)

$(DFILE2):	$(DFILE1)
	mkdir -p $(DESTDIR)
	cp -arn output/$(PREFIX)/bin $(DESTDIR)
	cp output/$(PREFIX)/sbin/* $(DESTDIR)/bin
	cp -arn output/$(PREFIX)/lib $(DESTDIR)
	rm -rf $(DESTDIR)/lib/python3
	mkdir -p $(DESTDIR)/share
	cp -arn output/$(PREFIX)/share/lirc $(DESTDIR)/share
	cp -arn output/$(PREFIX)/etc $(DESTDIR)

$(DFILE1):	$(BFILE)
	export PATH=$(TOOLCHAIN):$(PATH); make -C build install DESTDIR=$(TOPDIR)/output
	rm -f output/$(PREFIX)/lib/lirc/plugins/atilibusb.so

$(BFILE):	build/config.status

clean:
	rm -rf build output
