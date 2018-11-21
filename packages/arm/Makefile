#
# Default make builds avm toolchain and dropbear
#
# dropbear binaries are placed into dropbear/output and need to be
# copied manually to image (arm/mod/usr/local) on demand.
#
MODDIR=$(shell cd ../../arm/mod; pwd)

SUBDIRS = ffritz

BASE_SUBDIRS = buildroot dropbear

.PHONY:	$(SUBDIRS) $(BASE_SUBDIRS)

all:	$(SBUDIRS)

base:	$(BASE_SUBDIRS)

base-install:	base
	make -C dropbear install DESTDIR=$(MODDIR)

avm:
	make -C avm

dropbear:
	make -C dropbear 
	make -C dropbear install

buildroot:
	make -C buildroot

ffritz:
	make -C ffritz
