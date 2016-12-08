
PKGTOP	= $(shell cd ../..; pwd)
DLDIR	= $(PKGTOP)/dl

TOPDIR	    = $(shell pwd)
BUILDROOT   = $(shell cd ../buildroot/build; pwd)
TOOLCHAIN   = $(shell cd $(BUILDROOT)/output/host/usr/bin/; pwd)
DESTDIR	    = $(TOPDIR)/output
SYSROOT	    = $(BUILDROOT)/output/host/usr/i686-buildroot-linux-uclibc/sysroot

export PATH := $(TOOLCHAIN):$(PATH)
export CC   := i686-buildroot-linux-uclibc-gcc

URL	= $(shell cat $(PKGTOP)/url-lirc)
FILE	= $(DLDIR)/$(shell basename $(URL))

.PHONY:	build

all:	build/config.status
	make -C build

$(FILE):
	@cd $(DLDIR); wget $(URL)

build/configure:
	git clone git://git.code.sf.net/p/lirc/git build
#	@mkdir -p build
#	@cd build; tar xf $(FILE) --strip-components=1
	cd build; patch -p1 < ../curl_poll.patch
	cd build; ./autogen.sh
	
build/config.status:	build/configure
	@cd build; export PATH=$(TOOLCHAIN):$(PATH); ./configure --host=i686-buildroot-linux-uclibc --target=i686-buildroot-linux-uclibc
#	@cd build; export PATH=$(TOOLCHAIN):$(PATH); ./configure --target=i686-buildroot-linux-uclibc

clean:
	rm -rf build
