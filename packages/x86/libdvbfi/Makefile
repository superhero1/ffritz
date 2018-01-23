
BUILDROOT   = $(shell cd ../buildroot/build; pwd)

###############################################################################

TOPDIR=$(shell pwd)
TOOLCHAIN   = $(shell cd $(BUILDROOT)/output/host/usr/bin/; pwd)

TGTLIB=$(shell cd ../../../atom/squashfs-root/lib/; pwd)

CROSS_COMPILE=i686-buildroot-linux-uclibc-

WARN = -Wall -Wformat=0 -Werror

export PATH := $(TOOLCHAIN):$(PATH)

all:	libdvbfi.so

libdvbfi.so:	libdvbfi.c
	$(CROSS_COMPILE)gcc -std=c99 $(WARN) -fPIC -shared -Wl,-init,libinit -o $@ $<

clean:
	rm -f libdvbfi.so

