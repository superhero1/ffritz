include ../toolchain.mk

###############################################################################

TOPDIR=$(shell pwd)
TOOLCHAIN   = $(shell cd $(BUILDROOT)/output/host/usr/bin/; pwd)
DESTDIR	    = $(TOPDIR)/output
TGTLIB	    = ../../../arm/squashfs-root/lib
#TGTLIB	    = ../../../tmp/squashfs-root/lib/

CMN	    = ../../common/athtool

export PATH := $(TOOLCHAIN):$(PATH)
vpath %.c $(CMN)
CFLAGS += -I$(CMN)

###############################################################################

TARGETS	    = mdio-release pswtool pcount

## No longer supported in 6.8
#TARGETS    += athtool

all:	$(TGTLIB) $(TARGETS)


athtool:	athtool.o athtool_vlan.o athtool_counters.o athtool_atu.o counters.o
athtool:	LDFLAGS=-L$(TGTLIB) -lticc
mdio-release:	mdio-release.o
pswtool:	pswtool.o pswtool_counters.o counters.o
pswtool:	LDFLAGS=-L$(TGTLIB) -lticc
pcount:		pcount.o pp_counters.o netdev_counters.o pswtool_counters.o counters.o 
pcount:		LDFLAGS=-L$(TGTLIB) -lticc

$(TGTLIB):
	make -C ../../.. arm/squashfs-root

install:    all
	cp $(TARGETS) $(INSTDIR)/bin

clean:
	rm -f $(TARGETS) *.o *.d

include ../../rules.mk
-include $(wildcard *.d)
