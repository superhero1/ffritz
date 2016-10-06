TOPDIR	    = $(shell pwd)
PKGDIR	    = $(shell cd ..; pwd)

BINFILES    = $(shell cat binfiles) $(shell ls bin/*) src/sndlist src/usbplay
LIBFILES    = $(shell cat libfiles)
ETCFILES    = $(shell ls etc/*)

STAGEDIR    = $(TOPDIR)/output/ffritz

.PHONY:	src

all:	src ffritz.tar.gz

src:	
	make -C src

ffritz.tar.gz: $(BINFILES) $(LIBFILES)
	mkdir -p $(STAGEDIR)/bin
	mkdir -p $(STAGEDIR)/lib
	mkdir -p $(STAGEDIR)/etc
	mkdir -p $(STAGEDIR)/.mpd
	cp -a $(BINFILES) $(STAGEDIR)/bin
	cp -a $(LIBFILES) $(STAGEDIR)/lib
	cp -a $(ETCFILES) $(STAGEDIR)/etc
	cd output; tar cfz ../ffritz.tar.gz .
	
clean:
	rm -rf output ffritz.tar.gz
