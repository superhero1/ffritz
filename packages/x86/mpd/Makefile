#export CXX := i586-buildroot-linux-uclibc-cc

PKGTOP	= $(shell cd ../..; pwd)
DLDIR	= $(PKGTOP)/dl

TOPDIR	    = $(shell pwd)
BUILDROOT   = $(shell cd ../buildroot/build; pwd)
TOOLCHAIN   = $(shell cd $(BUILDROOT)/output/host/usr/bin/; pwd)
DESTDIR	    = $(TOPDIR)/output
SYSROOT	    = $(BUILDROOT)/output/host/usr/i686-buildroot-linux-uclibc/sysroot

export PATH := $(TOOLCHAIN):$(PATH)

URL	= $(shell cat $(PKGTOP)/url-mpd)
FILE	= $(DLDIR)/$(shell basename $(URL))

all:	build/config.status
	@make -C build 

$(FILE):
	@cd $(DLDIR); wget $(URL)

build/configure:    $(FILE)
	@mkdir -p build
	@cd build; tar xf $(FILE) --strip-components=1
	@cd build; patch -p1 < ../pipe_volume_ctrl.patch
	@touch build/configure

build/config.status:	build/configure
	@cd build; ./configure --host=i686-buildroot-linux-uclibc --target=i686-buildroot-linux-uclibc --with-zeroconf=no --disable-oss --disable-alsa --disable-pulse --enable-pipe-output --disable-icu --enable-glib --sysconfdir=/var/tmp --enable-mad --enable-lsr --enable-fifo --enable-flac --disable-mpg123 --enable-recorder-output --enable-sqlite --enable-vorbis --enable-aac

clean:
	rm -rf build
