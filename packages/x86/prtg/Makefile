
# Use AVM toolchain by default
#
ifeq ($(BUILDROOT),)
BUILDROOT   = $(shell cd ../buildroot/build; pwd)
endif

ifeq ($(BUILDROOT),)
$(error BUILDROOT (buildroot top directory) not defined)
endif

ifeq ($(CROSS_COMPILE),)
CROSS_COMPILE=i686-buildroot-linux-uclibc-
endif

###############################################################################

TOPDIR=$(shell pwd)
TOOLCHAIN   = $(shell cd $(BUILDROOT)/output/host/usr/bin/; pwd)
DESTDIR	    = $(TOPDIR)/output
TGTLIB	    = ../../../arm/squashfs-root/lib
#TGTLIB	    = ../../../tmp/squashfs-root/lib/

export PATH := $(TOOLCHAIN):$(PATH)

vpath %.c ../../arm/athtool

CFLAGS += -DATOM_BUILD -DPROC_FILE_ACCESS

###############################################################################

TARGETS	    = pcount

## No longer supported in 6.8
#TARGETS    += athtool

all:	$(TGTLIB) $(TARGETS)


pcount:		pcount.o netdev_counters.o counters.o athtool_counters.o

install:    all
	cp $(TARGETS) $(DESTDIR)/bin
	mkdir -p $(DESTDIR)/etc/prtg
	cp -ar scriptsxml $(DESTDIR)/etc/prtg
	cp run_prtg $(DESTDIR)/etc

clean:
	rm -f $(TARGETS) *.o *.d

doc:	html/index.html

html/index.html:    $(wildcard *.c *.h) Doxyfile
	doxygen Doxyfile

include ../../rules.mk
-include $(wildcard *.d)
ifeq ($(DESTDIR),)
DESTDIR=.
endif

