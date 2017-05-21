#export CXX := i586-buildroot-linux-uclibc-cc

PKGTOP	= $(shell cd ../..; pwd)
DLDIR	= $(PKGTOP)/dl

TOPDIR	    = $(shell pwd)
BUILDROOT   = $(shell cd ../buildroot/build; pwd)
TOOLCHAIN   = $(shell cd $(BUILDROOT)/output/host/usr/bin/; pwd)
DESTDIR	    = $(TOPDIR)/output
SYSROOT	    = $(BUILDROOT)/output/host/usr/i686-buildroot-linux-uclibc/sysroot

export PATH := $(TOOLCHAIN):$(PATH)
export CC   := i686-buildroot-linux-uclibc-gcc

URL	= $(shell cat $(PKGTOP)/url-shairport)
REPO	= $(DLDIR)/shairport

.PHONY:	build

all:	build/config.h
	make -C build

build/configure: $(REPO)
	rm -rf build
	git clone $(REPO) build
	cd build; patch -p1 < ../pipe_volfile.patch

$(REPO):
	git clone $(URL) $(REPO)

build/config.h:	build/configure
	@cd build; ./configure 

clean:
	rm -rf build
