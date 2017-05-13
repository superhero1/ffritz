TOPDIR	    = $(shell cd ..; pwd)
BUILDROOT   = $(shell cd ../../buildroot/build; pwd)
TOOLCHAIN   = $(shell cd $(BUILDROOT)/output/host/usr/bin/; pwd)
export PATH := $(TOOLCHAIN):$(PATH)
CC	    = i686-buildroot-linux-uclibc-gcc

CFLAGS += -I../../libmaru
LDFLAGS += ../../libmaru/libmaru.a -lusb-1.0 -pthread

.c.o:
	$(CC) -Wall -c $< -std=c99 $(CFLAGS)

all:	sndlist usbplay usbplayd ffdaemon

ffdaemon:	ffdaemon.o daemon.o
	$(CC) -o $@ $^ -std=c99 

sndlist:    sndlist.o
	$(CC) -o $@ $^ -std=c99 $(LDFLAGS)

usbplay:    usbplay.o
	$(CC) -o $@ $^ -std=c99 $(LDFLAGS)

usbplayd:    usbplayd.o daemon.o ../../libmaru/libmaru.a
	$(CC) -o $@ $^ -std=c99 $(LDFLAGS) -lsamplerate

clean:
	rm -f sndlist usbplay usbplayd *.o

