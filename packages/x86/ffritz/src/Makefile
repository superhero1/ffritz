TOPDIR	    = $(shell cd ..; pwd)
BUILDROOT   = $(shell cd ../../buildroot/build; pwd)
TOOLCHAIN   = $(shell cd $(BUILDROOT)/output/host/usr/bin/; pwd)
export PATH := $(TOOLCHAIN):$(PATH)
CC=i686-buildroot-linux-uclibc-gcc

all:	sndlist usbplay usbplayd su

su:    su.c
	$(CC) -Wall -o su su.c -std=c99 

sndlist:    sndlist.c
	$(CC) -Wall -o sndlist sndlist.c -std=c99 -I ../../libmaru ../../libmaru/libmaru.a -lusb-1.0

usbplay:    usbplay.c
	$(CC)  -Wall -o usbplay usbplay.c -std=c99 -I ../../libmaru ../../libmaru/libmaru.a -lusb-1.0

usbplayd:    usbplayd.c
	$(CC)  -Wall -o usbplayd usbplayd.c -std=c99 -I../../libmaru ../../libmaru/libmaru.a -lsamplerate -lusb-1.0

clean:
	rm -f sndlist usbplay usbplayd

