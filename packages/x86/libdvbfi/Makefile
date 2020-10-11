ARCHDIR=$(shell while test -f arch.mk && echo $$PWD && exit 0; test $$PWD != /; do cd ..; done)
include $(ARCHDIR)/arch.mk


###############################################################################

TGTLIB=$(SQ_ROOT)/lib/

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

