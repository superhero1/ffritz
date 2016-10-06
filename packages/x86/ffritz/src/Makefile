TOPDIR	    = $(shell cd ..; pwd)
BUILDROOT   = $(shell cd ../../buildroot/build; pwd)
TOOLCHAIN   = $(shell cd $(BUILDROOT)/output/host/usr/bin/; pwd)
export PATH := $(TOOLCHAIN):$(PATH)
CC=i686-buildroot-linux-uclibc-gcc

all:	sndlist usbplay

sndlist:    sndlist.c
	$(CC) -o sndlist sndlist.c -std=c99 -I ../../libmaru -L ../../libmaru -lmaru

usbplay:    usbplay.c
	$(CC) -o usbplay usbplay.c -std=c99 -I ../../libmaru -L ../../libmaru -lmaru


