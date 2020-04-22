ARCHDIR=$(shell while test -f arch.mk && echo $$PWD && exit 0; test $$PWD != /; do cd ..; done)
include $(ARCHDIR)/../paths.mk

DESTDIR	    = $(TOPDIR)/output

export PATH := $(TOOLCHAIN):$(PATH)

vpath %.c $(PKGTOP)/common/athtool

CFLAGS += -DATOM_BUILD -DPROC_FILE_ACCESS -I$(PKGTOP)/common/athtool

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
