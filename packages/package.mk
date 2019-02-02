## Options
#
# CONFTYPE: autoconf|autoreconf|legacy (default: autoconf)
# PKG_CONFIGURE_FLAGS: package specific configure flags
#
# DESTDIR: installation base (default: output)
#	   TOOLCHAIN to install to buildroot
# PKGNAME: Name of package   (default: dirname)
# URL:     URL of download tar file (default: content of url-PKGNAME)
#
# MAKE_OPTIONS: Options for make call
# MAKE_SUBDIR:  Subdir in source tree to call make in (with / prefix)
# MAKE_INSTALL_OPTIONS: Options for make install call
# NO_MAKE_INSTALL: If set, no make install is called
# INSTALL_BIN: files to put in DESTDIR/bin
# INSTALL_LIB: files/dirs to put in DESTDIR/lib
# INSTALL_ETC: files/dirs to put in DESTDIR/etc
# INSTALL_SHARE: files/dirs to put in DESTDIR/share
# FORCE_MAKE_INSTALL: force make install even if INSTALL_XX is set
# 


## Standard definitions 
#
include ../arch.mk

PKGTOP	= $(shell cd ../..; pwd)
DLDIR	= $(PKGTOP)/dl

TOPDIR	    = $(shell pwd)
BUILDROOT   = $(shell cd ../buildroot/build; pwd)
TOOLCHAIN   = $(shell cd $(BUILDROOT)/output/host/usr/bin/; pwd)
SYSROOT	    = $(BUILDROOT)/output/host/usr/i686-buildroot-linux-uclibc/sysroot
TGTDIR	    = $(BUILDROOT)/output/target


CC	:= $(CROSS)gcc
CONFIGURE_FLAGS	= --target=$(HOST) --host=$(HOST)

PATCHES=$(shell ls *.patch 2>/dev/null | sort)

## Configurable definitions 
#
ifeq ($(PKGNAME),)
PKGNAME     = $(shell basename `pwd`)
endif

ifeq ($(URL),)
URL	= $(shell test -r $(PKGTOP)/url-$(PKGNAME) && cat $(PKGTOP)/url-$(PKGNAME))
endif

ifeq ($(GIT),)
GIT	= $(shell test -r $(PKGTOP)/git-$(PKGNAME) && cat $(PKGTOP)/git-$(PKGNAME))
endif

ifneq ($(URL),)
FILE	= $(DLDIR)/$(shell basename $(URL))
else
ifneq ($(GIT),)
REPO	= $(DLDIR)/$(PKGNAME)
COMMIT  = $(shell test -r $(PKGTOP)/commit-$(PKGNAME) && cat $(PKGTOP)/commit-$(PKGNAME))
ifeq ($(COMMIT),)
COMMIT	= HEAD
endif
else
$(error No url/git file for $(PKGNAME))
endif
endif

all:

ifeq ($(CONFTYPE),)
CONFTYPE=autoconf
endif

AUTORECONF=
ifeq ($(CONFTYPE),autoconf)
ALL_DEP	= build/config.status
else
ifeq ($(CONFTYPE),autoreconf)
ALL_DEP = build/config.status
AUTORECONF=autoreconf -vfi
else
MAKE_OPTIONS += CC=$(CC)
ALL_DEP = build$(MAKE_SUBDIR)/Makefile
endif
endif

ifeq ($(FORCE_MAKE_INSTALL),)
ifneq ($(INSTALL_BIN),)
NO_MAKE_INSTALL=1
endif
ifneq ($(INSTALL_LIB),)
NO_MAKE_INSTALL=1
endif
ifneq ($(INSTALL_ETC),)
NO_MAKE_INSTALL=1
endif
ifneq ($(INSTALL_SHARE),)
NO_MAKE_INSTALL=1
endif
else
NO_MAKE_INSTALL=
endif

ifeq ($(DESTDIR),TOOLCHAIN)
DESTDIR	= $(TGTDIR)/bin
INST_TO_TOOLCHAIN=1
else
INST_TO_TOOLCHAIN=0
endif

ifeq ($(DESTDIR),)
DESTDIR	    = $(TOPDIR)/output
endif

_INST=$(shell echo "if [ -f $(1) ]; then install -vDC $(1) $(2)/`basename $(1)`; else mkdir -p $(2); cp -arv $(1) $(2); fi;")

#------------------------------------
ifneq ($(FILE),)
$(FILE):
	@cd $(DLDIR); wget $(URL)
endif

ifneq ($(REPO),)
$(REPO):
	git clone --bare $(GIT) $(REPO)
endif

ifneq ($(FILE),)
build:	$(FILE)
	@rm -rf build
	@mkdir -p build
	@cd build; tar xf $(FILE) --strip-components=1
	@cd build; for p in $(PATCHES); do echo Applying $$p ..; patch -p1 < ../$$p; done
	cd build; $(AUTORECONF)
else
build:	$(REPO)
	@rm -rf build
	@git clone $(REPO) build
	@cd build; git submodule update --init
	@cd build; git checkout $(COMMIT)
	@cd build; for p in $(PATCHES); do echo Applying $$p ..; patch -p1 < ../$$p; done
	cd build; $(AUTORECONF)
endif


all:	$(ALL_DEP)
	make -C build$(MAKE_SUBDIR) -j4 PATH=$(TOOLCHAIN):$(PATH) $(MAKE_OPTIONS)

build$(MAKE_SUBDIR)/Makefile:    build
	@test -f build$(MAKE_SUBDIR)/Makefile && touch build$(MAKE_SUBDIR)/Makefile

build/configure:    build

build/config.status:	build/configure
	cd build; export PATH=$(TOOLCHAIN):$(PATH); ./configure $(CONFIGURE_FLAGS) --build=x86_64-unknown-linux-gnu --prefix=/usr/local --exec-prefix=/usr --sysconfdir=/usr/local/etc --localstatedir=/var --program-prefix= --disable-doc --disable-docs --disable-documentation --disable-static --enable-shared $(PKG_CONFIGURE_FLAGS)
	touch $@

clean:
	rm -rf build


install: all
	@$(foreach f,$(INSTALL_BIN),$(call _INST,$(f),$(DESTDIR)/bin))
	@$(foreach f,$(INSTALL_LIB),$(call _INST,$(f),$(DESTDIR)/lib))
	@$(foreach f,$(INSTALL_ETC),$(call _INST,$(f),$(DESTDIR)/etc))
	@$(foreach f,$(INSTALL_SHARE),$(call _INST,$(f),$(DESTDIR)/share))
ifeq ($(NO_MAKE_INSTALL),)
ifneq ($(INST_TO_TOOLCHAIN),)
	make -C build install DESTDIR=$(DESTDIR) $(MAKE_INSTALL_OPTIONS)
else
	make -C build install DESTDIR=$(SYSROOT) $(MAKE_INSTALL_OPTIONS)
	make -C build install DESTDIR=$(TGTDIR) $(MAKE_INSTALL_OPTIONS)
endif
endif
