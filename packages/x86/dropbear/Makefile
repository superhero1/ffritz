PKGTOP	= $(shell cd ../..; pwd)
DLDIR	= $(PKGTOP)/dl

ifeq ($(BUILDROOT),)
BUILDROOT=$(shell cd ../buildroot/build/; pwd)
endif

TOPDIR	    = $(shell pwd)
TOOLCHAIN   = $(shell cd $(BUILDROOT)/output/host/usr/bin/; pwd)

ifeq ($(DESTDIR),)
    DESTDIR	    = $(TOPDIR)/output
endif

export PATH := $(TOOLCHAIN):$(PATH)

URL	= $(shell cat $(PKGTOP)/url-dropbear)
FILE	= $(DLDIR)/$(shell basename $(URL))

all:	build/config.status
	@make -C build PROGRAMS="dropbear dropbearkey dropbearconvert dbclient scp" SCPPROGRESS=1 CPPFLAGS=-DUSE_DEV_PTMX

install: all
	@make -C build PROGRAMS="dropbear dropbearkey dropbearconvert dbclient scp" SCPPROGRESS=1 install DESTDIR=$(DESTDIR)

$(FILE):
	@cd $(DLDIR); wget $(URL)

build/configure:    $(FILE)
	@mkdir -p build
	@cd build; tar xf $(FILE) --strip-components=1
	@cp options.h build/localoptions.h
	@touch build/configure

build/config.status:	build/configure
	@cd build; ./configure --host=i686-buildroot-linux-uclibc --target=i686-buildroot-linux-uclibc --with-gnu-ld --disable-nls --disable-pam --disable-syslog --enable-shadow --disable-lastlog --disable-utmp --disable-utmpx --disable-wtmp --disable-wtmpx --disable-loginfunc --disable-pututline --disable-pututxline --enable-bundled-libtom --disable-largefile --disable-zlib --disable-openpty --disable-harden 

clean:
	rm -rf build

distclean:
	rm -rf build
	rm -f $(FILE)
