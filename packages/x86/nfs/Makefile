PKGTOP	= $(shell cd ../..; pwd)
DLDIR	= $(PKGTOP)/dl

TOPDIR	    = $(shell pwd)
BUILDROOT   = $(shell cd ../buildroot/build; pwd)
TOOLCHAIN   = $(shell cd $(BUILDROOT)/output/host/usr/bin/; pwd)
DESTDIR	    = $(TOPDIR)/output
SYSROOT	    = $(BUILDROOT)/output/host/usr/i686-buildroot-linux-gnu/sysroot

URL	= $(shell cat $(PKGTOP)/url-nfs)
FILE	= $(DLDIR)/$(shell basename $(URL))

BFILE = build/fuse/fuse-nfs
DFILE = $(DESTDIR)/bin/fuse-nfs

all:	build/config.status
	@make -C build PATH=$(TOOLCHAIN):$(PATH)

$(FILE):
	@cd $(DLDIR); wget $(URL)

build/configure:    $(FILE)
	@rm -rf build
	@mkdir -p build
	@cd build; unzip $(FILE) && mv fuse-nfs-master/* .
	cd build; ./setup.sh

build/config.status:	build/configure
	@cd build; export PATH=$(TOOLCHAIN):$(PATH); ./configure --host=i686-buildroot-linux-gnu --target=i686-buildroot-linux-gnu --prefix=/

install: $(DFILE)

$(DFILE): $(BFILE)
	@make -C build PATH=$(TOOLCHAIN):$(PATH) install-exec DESTDIR=$(DESTDIR)

$(BFILE):	build/config.status
	@make -C build PATH=$(TOOLCHAIN):$(PATH)

clean:
	rm -rf build
