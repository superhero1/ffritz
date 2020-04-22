ARCHDIR=$(shell while test -f arch.mk && echo $$PWD && exit 0; test $$PWD != /; do cd ..; done)
include $(ARCHDIR)/../paths.mk

export PATH := $(TOOLCHAIN):$(PATH)
CC	    = armeb-buildroot-linux-uclibcgnueabi-gcc

vpath %.c $(TOPDIR)/common/ffdaemon

CFLAGS  += 
LDFLAGS +=

vpath %.c $(ARCHDIR)/ffritz/src

.c.o:
	$(CC) -Wall -c $< -std=c99 $(CFLAGS)

all:	ffdaemon

ffdaemon:	ffdaemon.o daemon.o
	$(CC) -o $@ $^ -std=c99 

clean:
	rm -f ffdaemon *.o

