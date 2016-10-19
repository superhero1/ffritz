
TARGETS = bin/test_fifo bin/test_enum

TOPDIR	    = $(shell pwd)
BUILDROOT   = $(shell cd ../../buildroot/build; pwd)
TOOLCHAIN   = $(shell cd $(BUILDROOT)/output/host/usr/bin/; pwd)
export PATH := $(TOOLCHAIN):$(PATH)
CC=i686-buildroot-linux-uclibc-gcc

TGT=/export/tmp/fritz/ffritz/packages/x86/buildroot/build/output/target/usr/lib
TLIB=$(TGT)/usr/lib
TINC=$(TGT)/usr/include

CFLAGS += -O3 -pthread -std=gnu99 -Wall -I.. -I$(TINC)
LDFLAGS += -pthread -L$(TLIB) -lusb-1.0 -lrt

all: $(TARGETS)

bin/test_fifo: test_fifo.o ../fifo.o ../eventfd.o
	mkdir -p bin
	$(CC) -o $@ $^ $(LDFLAGS)

bin/test_enum: test_enum.o ../fifo.o ../libmaru.o ../eventfd.o
	mkdir -p bin
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.o
	rm -f $(TARGETS)

.PHONY: all clean
