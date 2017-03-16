
# Use AVM toolchain by default
#
ifeq ($(BUILDROOT),)
BUILDROOT   = $(shell cd ../avm/build/prefix_sehr_sehr_sehr_sehr_sehr_sehr_sehr_sehr_sehr_lang_buildroot-2013.05/; pwd)
endif

ifeq ($(BUILDROOT),)
$(error BUILDROOT (buildroot top directory) not defined)
endif

ifeq ($(CROSS_COMPILE),)
CROSS_COMPILE=armeb-buildroot-linux-uclibcgnueabi-
endif

###############################################################################

TOPDIR=$(shell pwd)
TOOLCHAIN   = $(shell cd $(BUILDROOT)/output/host/usr/bin/; pwd)
DESTDIR	    = $(TOPDIR)/output

TGTLIB	    = $(shell cd ../../../arm/squashfs-root/lib/; pwd)
LIBS	    = -lticc

export PATH := $(TOOLCHAIN):$(PATH)

all:	athtool


athtool:    athtool.c
	$(CROSS_COMPILE)gcc -o $@ $< -L $(TGTLIB) $(LIBS)
