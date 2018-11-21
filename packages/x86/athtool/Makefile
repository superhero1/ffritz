include ../toolchain.mk

###############################################################################

TOPDIR=$(shell pwd)
TOOLCHAIN   = $(shell cd $(BUILDROOT)/output/host/usr/bin/; pwd)
DESTDIR	    = $(TOPDIR)/output

export PATH := $(TOOLCHAIN):$(PATH)

vpath %.c ../../common/athtool

CFLAGS += -DATOM_BUILD -DPROC_FILE_ACCESS -I../../common/athtool

###############################################################################

TARGETS	    = athtool pcount

all:	$(TARGETS)

athtool:	athtool.o athtool_vlan.o athtool_counters.o athtool_atu.o counters.o
pcount:		pcount.o netdev_counters.o athtool_counters.o counters.o 

install:    all
	cp $(TARGETS) $(INSTDIR)/bin

clean:
	rm -f $(TARGETS) *.o *.d

include ../../rules.mk
-include $(wildcard *.d)
