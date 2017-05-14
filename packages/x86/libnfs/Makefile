PKGTOP	= $(shell cd ../..; pwd)
DLDIR	= $(PKGTOP)/dl

TOPDIR	    = $(shell pwd)
BUILDROOT   = $(shell cd ../buildroot/build; pwd)
TOOLCHAIN   = $(shell cd $(BUILDROOT)/output/host/usr/bin/; pwd)
SYSROOT	    = $(BUILDROOT)/output/host/usr/i686-buildroot-linux-uclibc/sysroot
TARGET      = $(BUILDROOT)/output/target

BFILE = build/lib/.libs/libnfs.a
DFILE = $(DESTDIR)/usr/lib/libnfs.a

URL	= $(shell cat $(PKGTOP)/url-libnfs)
FILE	= $(DLDIR)/$(shell basename $(URL))

all:	build/config.status
	@make -C build PATH=$(TOOLCHAIN):$(PATH)
	@make install DESTDIR=$(SYSROOT)
	@make install DESTDIR=$(TARGET)

install: $(DFILE)

$(DFILE): $(BFILE)
	@make -C build PATH=$(TOOLCHAIN):$(PATH) install DESTDIR=$(DESTDIR)

$(BFILE):	build/config.status
	@make -C build PATH=$(TOOLCHAIN):$(PATH)

$(FILE):
	@cd $(DLDIR); wget $(URL)

build/configure:    $(FILE)
	@mkdir -p build
	@cd build; tar xf $(FILE) --strip-components=1
	@touch build/configure

build/config.status:	build/configure
	@cd build; export PATH=$(TOOLCHAIN):$(PATH); ./configure --host=i686-buildroot-linux-uclibc --target=i686-buildroot-linux-uclibc --prefix=/usr

clean:
	rm -rf build
