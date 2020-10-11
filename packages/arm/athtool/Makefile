ARCHDIR=$(shell while test -f arch.mk && echo $$PWD && exit 0; test $$PWD != /; do cd ..; done)
include $(ARCHDIR)/arch.mk

###############################################################################

DESTDIR	    = $(TOPDIR)/output

CMN	    = $(PKGTOP)/common/athtool

export PATH := $(TOOLCHAIN):$(PATH)
vpath %.c $(CMN)
CFLAGS += -I$(CMN) -fno-stack-protector

###############################################################################

TARGETS	    = pswtool pcount

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
