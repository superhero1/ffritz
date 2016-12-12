#
# Default make builds avm toolchain and dropbear
#
# dropbear binaries are placed into dropbear/output and need to be
# copied manually to image (arm/mod/usr/local) on demand.
#
SUBDIRS = avm dropbear ffritz

.PHONY:	$(SUBDIRS)

all:	$(SUBDIRS)

avm:
	make -C avm

dropbear:
	make -C dropbear install

