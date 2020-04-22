ARCHDIR=$(shell while test -f arch.mk && echo $$PWD && exit 0; test $$PWD != /; do cd ..; done)
include $(ARCHDIR)/../paths.mk

TGTDIR      = $(BUILDROOT)/output/target

BFILE = $(BUILDDIR)/lib/.libs/libnfs.a
DFILE = $(DESTDIR)/usr/lib/libnfs.a

URL	= $(shell cat $(PKGTOP)/url-libnfs)
FILE	= $(DLDIR)/$(shell basename $(URL))

all:	$(BUILDDIR)/config.status
	@make -C build PATH=$(TOOLCHAIN):$(PATH)
	@make install DESTDIR=$(SYSROOT)
	@make install DESTDIR=$(TGTDIR)

install: $(DFILE)

$(DFILE): $(BFILE)
	@make -C build PATH=$(TOOLCHAIN):$(PATH) install DESTDIR=$(DESTDIR)

$(BFILE):	$(BUILDDIR)/config.status
	@make -C build PATH=$(TOOLCHAIN):$(PATH)

$(FILE):
	@cd $(DLDIR); wget $(URL)

$(BUILDDIR)/configure:    $(FILE)
	@mkdir -p build
	@cd build; tar xf $(FILE) --strip-components=1
	@touch $(BUILDDIR)/configure

$(BUILDDIR)/config.status:	$(BUILDDIR)/configure
	@cd build; export PATH=$(TOOLCHAIN):$(PATH); ./configure --host=i686-buildroot-linux-uclibc --target=i686-buildroot-linux-uclibc --prefix=/usr

clean:
	rm -rf $(BUILDDIR)
