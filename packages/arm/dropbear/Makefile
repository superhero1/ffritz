
# Use AVM toolchain by default
#
ifeq ($(BUILDROOT),)
BUILDROOT   = $(shell cd ../avm/build/prefix_sehr_sehr_sehr_sehr_sehr_sehr_sehr_sehr_sehr_lang_buildroot-2013.05/; pwd)
endif

ifeq ($(BUILDROOT),)
$(error BUILDROOT (buildroot top directory) not defined)
endif

###############################################################################

TOPDIR=$(shell pwd)
TOOLCHAIN   = $(shell cd $(BUILDROOT)/output/host/usr/bin/; pwd)
ifeq ($(DESTDIR),)
DESTDIR	    = $(TOPDIR)/output
endif

export PATH := $(TOOLCHAIN):$(PATH)

URL	= $(shell cat ../../url-dropbear)
FILE	= ../../dl/$(shell basename $(URL))

all:	build/config.status
	@make -C build PROGRAMS="dropbear dropbearkey dropbearconvert dbclient scp" SCPPROGRESS=1 CPPFLAGS=-DUSE_DEV_PTMX

install:
	@make -C build PROGRAMS="dropbear dropbearkey dropbearconvert dbclient scp" SCPPROGRESS=1 install DESTDIR=$(DESTDIR)

$(FILE):
	@wget -N $(URL) -O $(FILE)

build/configure:    $(FILE)
	@mkdir -p build
	@cd build; tar xf ../$(FILE) --strip-components=1
	@cp options.h build/localoptions.h
	@touch build/configure

build/config.status:	build/configure
	@cd build; ./configure --host=armeb-buildroot-linux-uclibcgnueabi --target=armeb-buildroot-linux-uclibcgnueabi --with-gnu-ld --disable-nls --disable-pam --disable-syslog --enable-shadow --disable-lastlog --disable-utmp --disable-utmpx --disable-wtmp --disable-wtmpx --disable-loginfunc --disable-pututline --disable-pututxline --enable-bundled-libtom --disable-largefile --disable-zlib --disable-openpty --disable-harden

clean:
	rm -rf build

distclean:
	rm -rf build
	rm -f $(FILE)
