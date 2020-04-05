# requires: 
#	ARCHDIR		(packages/<arch>)
# optional:
#	BR_VERSION	buildroot version suffix
# provides:
#	BR_VERSION	Buildroot suffix
#	PKGTOP		real path of packages/
#	DLDIR		real path of download directory
#	TOPDIR		real path of current directory
#	BRBUILDDIR	real path of packages/arch/buildroot...	
#	BUILDROOT	real path of buildroot directory
#	TOOLCHAIN	host binaries (BR/output/host/usr/bin/)
#	SYSROOT		host binaries root
#	TGTDIR		buildroot target directory (BR/output/target)
#	CMNDIR		real path of common sources

-include $(ARCHDIR)/../../conf.mk
URL=
include $(ARCHDIR)/arch.mk

# default toolchain
#
ifeq ($(BR_VERSION),)
BR_VERSION  = -2019.05
endif

PKGTOP      = $(realpath $(ARCHDIR)/..)
CMNDIR      = $(PKGTOP)/common

DLDIR	    = $(PKGTOP)/dl
TOPDIR	    = $(shell pwd)
BRBUILDDIR  = $(realpath $(ARCHDIR)/buildroot$(BR_VERSION))
BUILDROOT   = $(BRBUILDDIR)/build
TOOLCHAIN   = $(realpath $(BUILDROOT)/output/host/usr/bin/)
SYSROOT	    = $(BUILDROOT)/output/host/usr/$(HOST)/sysroot
TGTDIR	    = $(BUILDROOT)/output/target
BUILDDIR    = build$(BR_VERSION)

TC_CHECK    = $(shell $(PKGTOP)/tccheck $(BUILDROOT))

