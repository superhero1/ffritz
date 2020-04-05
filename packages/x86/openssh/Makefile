ARCHDIR=$(shell while test -f arch.mk && echo $$PWD && exit 0; test $$PWD != /; do cd ..; done)
include $(ARCHDIR)/../paths.mk

TOPDIR	    = $(shell pwd)

ifeq ($(DESTDIR),)
    DESTDIR	    = $(TOPDIR)/output
endif

export PATH := $(TOOLCHAIN):$(PATH)

URL	= $(shell cat $(PKGTOP)/url-openssh)
FILE	= $(DLDIR)/$(shell basename $(URL))

all:	$(BUILDDIR)/config.status
	@make -C $(BUILDDIR)  

install:
	@make -C $(BUILDDIR) install DESTDIR=$(DESTDIR)

$(FILE):
	@cd $(DLDIR); wget $(URL)

$(BUILDDIR)/configure:    $(FILE)
	@mkdir -p $(BUILDDIR)
	@cd $(BUILDDIR); tar xf $(FILE) --strip-components=1
	@cd $(BUILDDIR); patch -p1 < ../openssh_add_ptmx_support.patch
	@touch $(BUILDDIR)/configure

$(BUILDDIR)/config.status:	$(BUILDDIR)/configure
	@cd $(BUILDDIR); ./configure --host=i686-buildroot-linux-uclibc --target=i686-buildroot-linux-uclibc --with-gnu-ld --disable-nls --disable-pam --disable-syslog --enable-shadow --disable-lastlog --disable-utmp --disable-utmpx --disable-wtmp --disable-wtmpx --disable-loginfunc --disable-pututline --disable-pututxline --enable-bundled-libtom --disable-largefile --disable-zlib --disable-openpty --disable-debug --disable-etc-default-login --disable-lastlog --disable-utmp --disable-utmpx --disable-wtmp --disable-wtmpx --without-bsd-auth --without-kerberos5 --without-pie --disable-static --prefix=/usr --sysconfdir=/etc/openssh CFLAGS=-DUSE_DEV_PTMX

clean:
	rm -rf $(BUILDDIR)

distclean:
	rm -rf $(BUILDDIR)
	rm -f $(FILE)
