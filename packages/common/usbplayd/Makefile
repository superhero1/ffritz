FFTOOLS ?= $(shell while test -d common/fftools && echo $$PWD/common/fftools && exit 0; test $$PWD != /; do cd ..; done)

CFLAGS  += -I$(FFTOOLS) -I.
LDFLAGS += -lusb-1.0 -pthread

ifeq ($(DESTDIR),)
DESTDIR = $(TOPDIR)/output
endif

TARGETS = usbplayd

LIBMARU = eventfd.o libmaru.o fifo.o

vpath %.c $(FFTOOLS)

.c.o:
	$(CC) -Wall -c $< -std=gnu99 -O2 -pthread -Wall $(CFLAGS)

all:	$(TARGETS)

install: all
	mkdir -p $(DESTDIR)/bin
	install $(TARGETS) $(DESTDIR)/bin

clean:
	rm -rf $(TARGETS) *.o

usbplayd:    usbplayd.o daemon.o $(LIBMARU)
	$(CC) -o $@ $^ -std=c99 $(LDFLAGS) -lsamplerate
