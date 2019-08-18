
BUILDROOT   = $(shell cd ../buildroot/build; pwd)

###############################################################################

TOPDIR=$(shell pwd)
TOOLCHAIN   = $(shell cd $(BUILDROOT)/output/host/usr/bin/; pwd)

TGTLIB=$(shell cd ../../../atom/squashfs-root/lib/; pwd)

CROSS_COMPILE=i686-buildroot-linux-gnu-

WARN = -Wall -Wformat=0 -Werror

#LIBS=-L$(TGTLIB) -lsystem_utils -lismd_demux -lismd_clock_recovery -lismd_core -lismdmessage -lsven -lpal -losal -lsystem_utils -lclock_control -lplatform_config


export PATH := $(TOOLCHAIN):$(PATH)

all:	libdvbfi.so

libdvbfi.so:	libdvbfi.c Makefile
	$(CROSS_COMPILE)gcc -O -std=c99 $(WARN) -fPIC -shared -Wl,-init,libinit -o $@ $< $(LIBS)

clean:
	rm -f libdvbfi.so

install: all
	@mkdir -p $(DESTDIR)/lib
	@mkdir -p $(DESTDIR)/etc/init.d
	@cp libdvb.rc run_cableinfo $(DESTDIR)/etc
	@cp run_cableinfo $(DESTDIR)/etc/init.d/30cableinfo
	@cp libdvbfi.so $(DESTDIR)/lib

