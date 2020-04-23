ARCHDIR=$(shell while test -f arch.mk && echo $$PWD && exit 0; test $$PWD != /; do cd ..; done)
include $(ARCHDIR)/../paths.mk

export PATH := $(TOOLCHAIN):$(PATH)
CC	    = i686-buildroot-linux-gnu-gcc

CFLAGS += -I$(ARCHDIR)/libmaru -I$(ARCHDIR) -I$(CMNDIR)/ffdaemon
LDFLAGS += -lusb-1.0 -pthread

ifeq ($(DESTDIR),)
DESTDIR = $(TOPDIR)/output
endif

TARGETS = sndlist usbplay usbplayd ffdaemon

LIBMARU = eventfd.o libmaru.o fifo.o

vpath %.c $(CMNDIR)/ffdaemon ../ $(ARCHDIR)/libmaru

.c.o:
	$(CC) -Wall -c $< -std=c99 -O2 -pthread -Wall $(CFLAGS)

all:	$(BUILDDIR)
	make -C $(BUILDDIR) tools

$(BUILDDIR):
	mkdir -p $(BUILDDIR)
	cd $(BUILDDIR); ln -sf ../Makefile .

install: all
	mkdir -p $(DESTDIR)/bin
	cd $(BUILDDIR); cp -a $(TARGETS) $(DESTDIR)/bin

clean:
	rm -rf $(BUILDDIR)

libmaru.o fifo.o:	CFLAGS += -std=gnu99

################

tools:	$(TARGETS)
	mkdir -p bin
	cp -a $(TARGETS) bin

ffdaemon:	ffdaemon.o daemon.o
	$(CC) -o $@ $^ -std=c99 

sndlist:    sndlist.o $(LIBMARU)
	$(CC) -o $@ $^ -std=c99 $(LDFLAGS)

usbplay:    usbplay.o $(LIBMARU)
	$(CC) -o $@ $^ -std=c99 $(LDFLAGS)

usbplayd:    usbplayd.o daemon.o $(LIBMARU)
	$(CC) -o $@ $^ -std=c99 $(LDFLAGS) -lsamplerate
