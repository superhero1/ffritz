TARGET := libmaru.so.0.1
SONAME := libmaru.so.0
SONAME_SHORT := libmaru.so

PREFIX = /usr/local

CC=i686-buildroot-linux-uclibc-gcc
TGT=/export/tmp/fritz/ffritz/packages/x86/buildroot/build/output/target/usr/lib
TLIB=$(TGT)/usr/lib
TINC=$(TGT)/usr/include

CFLAGS += -O2 -g -pthread -fPIC -std=gnu99 -Wall -I.. -I$(TINC)
LDFLAGS += -shared -pthread -fPIC -lrt -L$(TLIB) -lusb-1.0 -lfuse -Wl,-no-undefined -Wl,-soname,$(SONAME)

SOURCES := $(wildcard *.c)
OBJECTS := $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f $(OBJECTS)
	rm -f $(TARGET)

install:
	install -m755 $(TARGET) $(PREFIX)/lib
	mkdir -p $(PREFIX)/include/libmaru 2>/dev/null || /bin/true
	install -m644 libmaru.h $(PREFIX)/include/libmaru
	install -m644 fifo.h $(PREFIX)/include/libmaru
	ln -sf $(TARGET) $(PREFIX)/lib/$(SONAME)
	ln -sf $(TARGET) $(PREFIX)/lib/$(SONAME_SHORT)
	cat libmaru.pc.in | sed -e 's|PREFIXSTUB|$(PREFIX)|' > libmaru.pc
	mkdir -p $(PREFIX)/lib/pkgconfig 2>/dev/null || /bin/true
	install -m644 libmaru.pc $(PREFIX)/lib/pkgconfig && rm -f libmaru.pc

.PHONY: all clean install

