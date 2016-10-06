TOPDIR	    = $(shell pwd)
PKGDIR	    = $(shell cd ..; pwd)

BINFILES    = $(shell cat binfiles) $(shell ls bin/*) src/sndlist src/usbplay
LIBFILES    = $(shell cat libfiles)
ETCFILES    = $(shell ls etc/*)

STAGEDIR    = $(TOPDIR)/output/ffritz

VERSION     = $(shell cat version)

IMAGE       = ffritz-x86-$(VERSION).tar.gz

.PHONY:	src

all:	src $(IMAGE)

src:	
	make -C src

$(IMAGE): $(BINFILES) $(LIBFILES)
	mkdir -p $(STAGEDIR)/bin
	mkdir -p $(STAGEDIR)/lib
	mkdir -p $(STAGEDIR)/etc
	mkdir -p $(STAGEDIR)/.mpd
	cp -a $(BINFILES) $(STAGEDIR)/bin
	cp -a $(LIBFILES) $(STAGEDIR)/lib
	cp -a $(ETCFILES) $(STAGEDIR)/etc
	cp ../../../MPD.txt output/README.txt
	strip $(STAGEDIR)/bin/* || exit 0
	cd output; tar cfz ../$(IMAGE) .
	
clean:
	rm -rf output $(IMAGE)
