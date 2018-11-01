TOPDIR	    = $(shell cd ..; pwd)
BUILDROOT   = $(shell cd ../../buildroot/build; pwd)
TOOLCHAIN   = $(shell cd $(BUILDROOT)/output/host/usr/bin/; pwd)
export PATH := $(TOOLCHAIN):$(PATH)
CC	    = armeb-buildroot-linux-uclibcgnueabi-gcc

CFLAGS  += 
LDFLAGS +=

vpath %.c ../../../x86/ffritz/src

.c.o:
	$(CC) -Wall -c $< -std=c99 $(CFLAGS)

all:	ffdaemon

ffdaemon:	ffdaemon.o daemon.o
	$(CC) -o $@ $^ -std=c99 

clean:
	rm -f ffdaemon *.o

