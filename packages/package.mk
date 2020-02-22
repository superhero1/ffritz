## Options
#
# CONFTYPE: autoconf|autoreconf|legacy (default: autoconf)
# PKG_CONFIGURE_FLAGS: package specific configure flags
#
# DESTDIR: installation base (default: output)
#	   TOOLCHAIN to install to buildroot
# PKGNAME: Name of package   (default: dirname)
# URL:     URL of download tar file (default: content of url-pkgname)
# GIT:     URL of git repository (default: content of git-pkgname)
#					 You might also want to specify the tag in COMMIT / commit-pkgname.
#
# MAKE_OPTIONS: Options for make call
# MAKE_SUBDIR:  Subdir in source tree to call make in (with / prefix)
# MAKE_INSTALL_TGT: Target(s) for make install call (default: install)
# MAKE_INSTALL_OPTIONS: Options for make install call
# NO_MAKE_INSTALL: If set, no make install is called
# INSTALL_BIN: files to put in DESTDIR/bin, don't call make install
# INSTALL_LIB: files/dirs to put in DESTDIR/lib, don't call make install
# INSTALL_ETC: files/dirs to put in DESTDIR/etc, don't call make install
# INSTALL_SHARE: files/dirs to put in DESTDIR/share, don't call make install
# FORCE_MAKE_INSTALL: force make install even if INSTALL_XX is set
# CUSTOM_DEP: Own dependencies for preparing build
# BUILDDIR: Name of build sub-directory (default: build)
# 

ARCHDIR=$(shell while test -f arch.mk && echo $$PWD && exit 0; [[ $PWD != / ]]; do cd ..; done)

## Standard definitions 
#
include $(ARCHDIR)/arch.mk

PKGTOP	= $(shell cd $(ARCHDIR)/..; pwd)
DLDIR	= $(PKGTOP)/dl

TOPDIR	    = $(shell pwd)
BUILDROOT   = $(shell cd $(ARCHDIR)/buildroot/build; pwd)
TOOLCHAIN   = $(shell cd $(BUILDROOT)/output/host/usr/bin/; pwd)
SYSROOT	    = $(BUILDROOT)/output/host/usr/$(HOST)/sysroot
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
REPO	= $(DLDIR)/$(PKGNAME)_git
ifeq ($(COMMIT),)
COMMIT  = $(shell test -r $(PKGTOP)/commit-$(PKGNAME) && cat $(PKGTOP)/commit-$(PKGNAME))
ifeq ($(COMMIT),)
COMMIT	= HEAD
endif
endif
else
$(error No url/git file for $(PKGNAME))
endif
endif

all: all-pkg

ifeq ($(CONFTYPE),)
CONFTYPE=autoconf
endif

AUTORECONF=
ifeq ($(CONFTYPE),autoconf)
ALL_DEP	= $(BUILDDIR)/config.status
else
ifeq ($(CONFTYPE),autoreconf)
ALL_DEP = $(BUILDDIR)/config.status
AUTORECONF=autoreconf -vfi
else
MAKE_OPTIONS += CC=$(CC)
ALL_DEP = $(BUILDDIR)$(MAKE_SUBDIR)/Makefile
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
INST_TO_TOOLCHAIN=1
else
INST_TO_TOOLCHAIN=
endif

ifeq ($(DESTDIR),)
DESTDIR	    = $(TOPDIR)/output
endif

ifeq ($(MAKE_INSTALL_TGT),)
MAKE_INSTALL_TGT = install
endif

ifeq ($(BUILDDIR),)
BUILDDIR=build
endif

_INST=$(shell echo "if [ -f $(1) ]; then install -vDC $(1) $(2)/`basename $(1)`; else mkdir -p $(2); cp -arv $(1) $(2); fi;")

#------------------------------------
ifneq ($(FILE),)
$(FILE):
	@cd $(DLDIR); wget $(URL)
	if [ -r $(PKGTOP)/sha-$(PKGNAME) ]; then cd $(DLDIR); sha256sum -c $(PKGTOP)/sha-$(PKGNAME); fi
endif

ifneq ($(REPO),)
$(REPO):
	git clone --bare $(GIT) $(REPO)
endif

ifneq ($(FILE),)
$(BUILDDIR):	$(FILE)
	@rm -rf $(BUILDDIR)
	@mkdir -p $(BUILDDIR)
	@cd $(BUILDDIR); tar xf $(FILE) --strip-components=1
	@cd $(BUILDDIR); for p in $(PATCHES); do echo Applying $$p ..; patch -p1 < ../$$p || exit 1; done
	cd $(BUILDDIR); $(AUTORECONF)
else
$(BUILDDIR):	$(REPO)
	@rm -rf $(BUILDDIR)
	@git clone $(REPO) $(BUILDDIR)
	@cd $(BUILDDIR); git submodule update --init
	@cd $(BUILDDIR); git checkout $(COMMIT)
	@cd $(BUILDDIR); for p in $(PATCHES); do echo Applying $$p ..; patch -p1 < ../$$p || exit 1; done
	cd $(BUILDDIR); $(AUTORECONF)
endif


all-pkg:	$(ALL_DEP) $(CUSTOM_DEP)
	PATH=$(TOOLCHAIN):$(PATH) make -C $(BUILDDIR)$(MAKE_SUBDIR) $(MAKE_OPTIONS)

$(BUILDDIR)$(MAKE_SUBDIR)/Makefile:    $(BUILDDIR)
	@test -f $(BUILDDIR)$(MAKE_SUBDIR)/Makefile && touch $(BUILDDIR)$(MAKE_SUBDIR)/Makefile

$(BUILDDIR)/configure:    $(BUILDDIR)

$(BUILDDIR)/config.status:	$(BUILDDIR)/configure
	cd $(BUILDDIR); export PATH=$(TOOLCHAIN):$(PATH); ./configure $(CONFIGURE_FLAGS) --build=x86_64-unknown-linux-gnu --prefix=/usr/local --exec-prefix=/usr --sysconfdir=/usr/local/etc --localstatedir=/var --program-prefix= --disable-doc --disable-docs --disable-documentation --disable-static --enable-shared $(PKG_CONFIGURE_FLAGS)
	touch $@

clean:	clean-pkg 
 
clean-pkg:
	rm -rf $(BUILDDIR)
	rm -f .*.stamp

install: install-pkg

install-pkg: all
	@$(foreach f,$(INSTALL_BIN),$(call _INST,$(f),$(DESTDIR)/bin))
	@$(foreach f,$(INSTALL_LIB),$(call _INST,$(f),$(DESTDIR)/lib))
	@$(foreach f,$(INSTALL_ETC),$(call _INST,$(f),$(DESTDIR)/etc))
	@$(foreach f,$(INSTALL_SHARE),$(call _INST,$(f),$(DESTDIR)/share))
ifeq ($(NO_MAKE_INSTALL),)
ifeq ($(INST_TO_TOOLCHAIN),)
	PATH=$(TOOLCHAIN):$(PATH) make -C $(BUILDDIR)$(MAKE_SUBDIR) $(MAKE_INSTALL_TGT) DESTDIR=$(DESTDIR) $(MAKE_INSTALL_OPTIONS)
else
	PATH=$(TOOLCHAIN):$(PATH) make -C $(BUILDDIR)$(MAKE_SUBDIR) $(MAKE_INSTALL_TGT) DESTDIR=$(SYSROOT) $(MAKE_INSTALL_OPTIONS)
	PATH=$(TOOLCHAIN):$(PATH) make -C $(BUILDDIR)$(MAKE_SUBDIR) $(MAKE_INSTALL_TGT) DESTDIR=$(TGTDIR) $(MAKE_INSTALL_OPTIONS)
endif
endif
