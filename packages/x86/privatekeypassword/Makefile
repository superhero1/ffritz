
PKGTOP	= $(shell cd ../..; pwd)
DLDIR	= $(PKGTOP)/dl

TOPDIR	    = $(shell pwd)
BUILDROOT   = $(shell cd ../buildroot/build; pwd)
TOOLCHAIN   = $(shell cd $(BUILDROOT)/output/host/usr/bin/; pwd)
SYSROOT	    = $(BUILDROOT)/output/host/usr/i686-buildroot-linux-uclibc/sysroot

PREFIX=/usr/local

export PATH := $(TOOLCHAIN):$(PATH)
export CC   := i686-buildroot-linux-uclibc-gcc

URL	= $(shell cat $(PKGTOP)/url-privatekeypassword)
REPO	= $(DLDIR)/privatekeypassword

.PHONY:	build

all:	build/Makefile
	make -C build

build/Makefile: $(REPO)
	git clone $(REPO) build

$(REPO):
	git clone $(URL) $(REPO)
	
BFILE = build/privatekeypassword
DFILE = $(DESTDIR)/bin/privatekeypassword

install: $(DFILE)

$(DFILE):	$(BFILE)
	mkdir -p $(DESTDIR)/bin
	mkdir -p $(DESTDIR)/lib
	cp -a build/privatekeypassword $(DESTDIR)/bin
	cp -a build/libprivatekeypassword.so $(DESTDIR)/lib

$(BFILE):	build/Makefile
	make -C build

clean:
	rm -rf build output
