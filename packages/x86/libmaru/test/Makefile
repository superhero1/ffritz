ARCHDIR=$(shell while test -f arch.mk && echo $$PWD && exit 0; test $$PWD != /; do cd ..; done)
include $(ARCHDIR)/arch.mk

TARGETS = bin/test_fifo bin/test_enum

export PATH := $(TOOLCHAIN):$(PATH)
CC=i686-buildroot-linux-gnu-gcc

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
