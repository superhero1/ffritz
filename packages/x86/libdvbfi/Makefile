
BUILDROOT   = $(shell cd ../buildroot/build; pwd)

###############################################################################

TOPDIR=$(shell pwd)
TOOLCHAIN   = $(shell cd $(BUILDROOT)/output/host/usr/bin/; pwd)

TGTLIB=$(shell cd ../../../atom/squashfs-root/lib/; pwd)

CROSS_COMPILE=i686-buildroot-linux-uclibc-

WARN = -Wall -Wformat=0 -Werror

#LIBS=-L$(TGTLIB) -lsystem_utils -lismd_demux -lismd_clock_recovery -lismd_core -lismdmessage -lsven -lpal -losal -lsystem_utils -lclock_control -lplatform_config


export PATH := $(TOOLCHAIN):$(PATH)

all:	libdvbfi.so cableinfo_p

libdvbfi.so:	libdvbfi.c Makefile
	$(CROSS_COMPILE)gcc -O -std=c99 $(WARN) -fPIC -shared -Wl,-init,libinit -o $@ $< $(LIBS)

cableinfo_p:	$(TGTLIB)/../sbin/cableinfo
	@./spatch $< $@ libdvbif.so libdvbfi.so

clean:
	rm -f libdvbfi.so


