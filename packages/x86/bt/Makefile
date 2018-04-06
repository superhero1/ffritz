PKGTOP	= $(shell cd ../..; pwd)
DLDIR	= $(PKGTOP)/dl

URL	= $(shell cat $(PKGTOP)/url-btstack)
COMMIT	= $(shell cat $(PKGTOP)/commit-btstack)
REPO	= $(DLDIR)/btstack

TARGET	= build/port/libusb/a2dp_sink_demo

.PHONY:	$(TARGET)

all: 	build $(TARGET)

$(TARGET):
	cd build/port/libusb; make a2dp_sink_demo

$(REPO):
	git clone --bare $(URL) $(REPO)

build:	$(REPO)
	rm -rf build
	git clone $(REPO) build
	cd build; git reset --hard $(COMMIT)
	cd build; patch -p1 < ../ffritz.patch

install: all
	mkdir -p $(DESTDIR)/bin
	cp $(TARGET) $(DESTDIR)/bin

clean:
	rm -rf build
	

