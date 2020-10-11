# optional:
#	BR_VERSION	buildroot version suffix
#	ARCHDIR		(packages/<arch>)
# provides:
#	BR_VERSION	Buildroot suffix
#	REPODIR		repository top directory
#	PKGTOP		real path of packages/
#	DLDIR		real path of download directory
#	TOPDIR		real path of current directory
#	CMNDIR		real path of common sources
# If ARCHDIR:
#	BRBUILDDIR	real path of packages/arch/buildroot...	
#	BUILDROOT	real path of buildroot directory
#	TOOLCHAIN	host binaries (BR/output/host/usr/bin/)
#	SYSROOT		host binaries root
#	TGTDIR		buildroot target directory (BR/output/target)
#       PLAT_CPU        FritzOS staging topdir for CPU (arm/atom)
#       PLAT_OSBASE     Staging topdir for FritzOS squashfs-root
#	PLAT_OVERLAY	ffritz overlay repo for FritzOS
# FritzOS platform staging:
#       PLAT_BASE       Base directory of system platform (puma6/7)

ifeq ($(ARCHDIR),)
REPODIR := $(shell while test -f mk/ffconfig.mk && echo $$PWD && exit 0; test $$PWD != /; do cd ..; done)
else
REPODIR     = $(realpath $(ARCHDIR)/../..)
endif

include $(REPODIR)/mk/topcfg.mk
URL=

# default toolchain
#
ifeq ($(BR_VERSION),)
BR_VERSION  = -2019.05
endif

PKGTOP      := $(REPODIR)/packages
TOOLDIR     := $(REPODIR)/tools
CMNDIR      := $(PKGTOP)/common
DLDIR	    := $(PKGTOP)/dl
TOPDIR	    = $(shell pwd)
BUILDDIR    = build$(BR_VERSION)
TC_CHECK    = $(shell $(PKGTOP)/tccheck $(BUILDROOT))

PLAT_BASE   = $(REPODIR)

URL_REPO    = $(PKGTOP)/external

WGET_TOOL   = $(TOOLDIR)/_wget

ifneq ($(ARCHDIR),)

BRBUILDDIR  = $(ARCHDIR)/buildroot$(BR_VERSION)
BUILDROOT   = $(BRBUILDDIR)/build
SYSROOT	    = $(BUILDROOT)/output/host/usr/$(HOST)/sysroot
TGTDIR	    = $(BUILDROOT)/output/target

TOOLCHAIN   = $(BUILDROOT)/output/host/opt/ext-toolchain/usr/bin/
ifeq ($(realpath $(TOOLCHAIN)),)
TOOLCHAIN   = $(BUILDROOT)/output/host/usr/bin/
endif

export PKG_CONFIG_PATH = $(SYSROOT)/usr/lib/pkgconfig

ifeq ($(ARCH),arm)
PLAT_CPU     := $(PLAT_BASE)/arm
else
PLAT_CPU     := $(PLAT_BASE)/atom
endif
PLAT_OSBASE  := $(PLAT_CPU)/suqashfs-root
PLAT_OVERLAY := $(PLAT_CPU)/mod

endif

URL_PREFIX  = $(URL_REPO)/url-
SHA_PREFIX  = $(URL_REPO)/sha-
GIT_PREFIX  = $(URL_REPO)/git-
COMM_PREFIX = $(URL_REPO)/commit-
URLGET      = $(shell test -r $(URL_PREFIX)$(1) && cat $(URL_PREFIX)$(1) || false)
SHAGET      = $(shell test -r $(SHA_PREFIX)$(1) && cat $(SHA_PREFIX)$(1))
GITGET      = $(shell test -r $(GIT_PREFIX)$(1) && cat $(GIT_PREFIX)$(1) || false)
COMMGET     = $(shell test -r $(COMM_PREFIX)$(1) && cat $(COMM_PREFIX)$(1))

##################################################

_SILENT=-s
_V_FLAG=
ifneq ($(V),)
_SILENT=
_V_FLAG=-v
endif
ifeq ($(V),2)
_VV_FLAG=-v
endif

# Some tools
#
SYMLINK  := ln $(_V_FLAG) -s
INSTALL  := install $(_V_FLAG)
SUBMAKE  := make $(_SILENT) -C
SUBMAKEK := make -k $(_SILENT) -C
##CHECKDIR  = $(shell d=`readlink -f $(1)|grep "^$(REPODIR)"`; test ! -z $$d && test -d $$d -a -w $$d)
##RMDIR     = $(shell d=`readlink -f $(1)|grep "^$(REPODIR)"`; test ! -z $$d && test -d $$d -a -w $$d && echo rm -rf $(_VV_FLAG) $$d)
##GITCLEAN  = $(shell d=`readlink -f $(1)|grep "^$(REPODIR)"`; test ! -z $$d && test -d $$d -a -w $$d && echo git clean -qfdx)
CHECKDIR  = $(shell d=`readlink -f $(1)`; test ! -z $$d && test -d $$d -a -w $$d)
RMDIR     = $(shell d=`readlink -f $(1)`; test ! -z $$d && test -d $$d -a -w $$d && echo rm -rf $(_VV_FLAG) $$d)
GITCLEAN  = $(shell d=`readlink -f $(1)`; test ! -z $$d && test -d $$d -a -w $$d && echo git clean -qfdx)
NEWEST_FILE = $(shell find $(1) -type f -exec ls -tr '{}' +|tail -1)


# List all executables, except .so
# Based on LS_ALL, list all ELF files
#
LS_ALL=$(shell find -L $(1) -maxdepth 1 -executable -type f ! -name "*\.so*" | sort)
LS_ELF_ALL=$(shell echo $(call LS_ALL,$(1)) | xargs file -L | grep ELF | sed -e 's/:.*//')

###########################################################################
# Some macros for ffwrap
#
# List all executables in $1, except .so and ffwrap
LS=$(shell find -L $(1) -maxdepth 1 -executable -type f ! -name "*\.so*" -a ! -name "ffwrap*" | sort)

# based on it LS:
#
# list ELF files in $1
LS_ELF=$(shell echo $(call LS,$(1)) | xargs file -L | grep ELF | sed -e 's/:.*//')

# list non-ELF files in $1
LS_NOT_ELF=$(shell echo $(call LS,$(1)) | xargs file -L | grep -v ELF | sed -e 's/:.*//')
############################################################################

# echo: ln -s $1 $2;
LINK=$(shell echo $(SYMLINK) $(1) $(2)\;)

# $1 list
# $2 destdir
# $3 link target file
LINK_TO_ONE=$(foreach f,$(1),$(call LINK,$(3),$(2)/$(shell basename $f)))

# $1 list
# $2 destdir
# $3 link target dir
LINK_TO_DIR=$(foreach f,$(1),$(call LINK,$(3)/$(shell basename $f),$(2)/$(shell basename $f)))

# $1 url
# $2 target dir or file
# $3 sha256sum file (optional)
WGET = $(shell echo $(WGET_TOOL) $(1) $(2) $(3))

# $1 external component name
# $2 optional download file/path
WGET_EXT  = $(call WGET,$(call URLGET,$(1)),$(DLDIR),$(SHA_PREFIX)$(1))
WGET_EXT2 = $(call WGET,$(call URLGET,$(1)),$(2),$(SHA_PREFIX)$(1))
