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

export PATH := $(TOOLCHAIN):$(PATH)

vpath %.c ../../arm/athtool

CFLAGS += -DATOM_BUILD -DPROC_FILE_ACCESS -I../../arm/athtool

###############################################################################

TARGETS	    = athtool pcount

all:	$(TARGETS)

athtool:	athtool.o athtool_vlan.o athtool_counters.o athtool_atu.o counters.o
pcount:		pcount.o netdev_counters.o athtool_counters.o counters.o 

install:    all
	cp $(TARGETS) $(INSTDIR)/bin

clean:
	rm -f $(TARGETS) *.o *.d

doc:	html/index.html

html/index.html:    $(wildcard *.c *.h) Doxyfile
	doxygen Doxyfile

include ../../rules.mk
-include $(wildcard *.d)
