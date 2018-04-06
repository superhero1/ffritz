#
# Default make builds avm toolchain and dropbear
#
# dropbear binaries are placed into dropbear/output and need to be
# copied manually to image (arm/mod/usr/local) on demand.
#
MODDIR=$(shell cd ../../arm/mod; pwd)

SUBDIRS = avm dropbear ffritz

BASE_SUBDIRS = avm dropbear

.PHONY:	$(SUBDIRS)

all:	$(SUBDIRS)

base:	$(BASE_SUBDIRS)

base-install:	base
	make -C dropbear install DESTDIR=$(MODDIR)

avm:
	make -C avm

dropbear:
	make -C dropbear 
	make -C dropbear install

ffritz:
	make -C ffritz
