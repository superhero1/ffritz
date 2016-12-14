
PKGTOP	= $(shell cd ../..; pwd)
DLDIR	= $(PKGTOP)/dl

TOPDIR	    = $(shell pwd)
BUILDROOT   = $(shell cd ../buildroot/build; pwd)
TOOLCHAIN   = $(shell cd $(BUILDROOT)/output/host/usr/bin/; pwd)
SYSROOT	    = $(BUILDROOT)/output/host/usr/i686-buildroot-linux-uclibc/sysroot

export PATH := $(TOOLCHAIN):$(PATH)
export CC   := i686-buildroot-linux-uclibc-gcc

URL	= $(shell cat $(PKGTOP)/url-lirc)
COMMIT	= $(shell cat $(PKGTOP)/commit-lirc)
FILE	= $(DLDIR)/$(shell basename $(URL))

.PHONY:	build

all:	build/config.status
	make -C build
	mkdir -p $(TOPDIR)/output
	export PATH=$(TOOLCHAIN):$(PATH); make -C build install DESTDIR=$(TOPDIR)/output
	rm -rf output/usr/local/share/doc output/usr/local/share/man output/usr/local/lib/python3

$(FILE):
	@cd $(DLDIR); wget $(URL)

build/configure:
	git clone git://git.code.sf.net/p/lirc/git build
	cd build; git reset $(COMMIT)
#	@mkdir -p build
#	@cd build; tar xf $(FILE) --strip-components=1
	cd build; patch -p1 < ../curl_poll.patch
	cd build; ./autogen.sh
	
build/config.status:	build/configure
	@cd build; export PATH=$(TOOLCHAIN):$(PATH); ./configure --host=i686-buildroot-linux-uclibc --target=i686-buildroot-linux-uclibc --with-driver=userspace

install:
	mkdir -p $(DESTDIR)
	cp -arnv output/usr/local/* $(DESTDIR)

clean:
	rm -rf build
