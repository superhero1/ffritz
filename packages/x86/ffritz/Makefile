TOPDIR	    = $(shell pwd)
PKGDIR	    = $(shell cd ..; pwd)

BINFILES    = ../mpd/build/src/mpd bin/runmpd bin/usb_pipe_play src/sndlist src/usbplay
LIBFILES    = $(shell ls ../buildroot/build/output/target/usr/lib/lib*so*) $(shell ls ../libmaru/lib*so*)
ETCFILES    = etc/ffritz-env.sh etc/mpd.conf

STAGEDIR    = $(TOPDIR)/output/ffritz

VERSION     = $(shell cat version)

IMAGE       = ffritz-x86-$(VERSION).tar.gz

.PHONY:	src

all:	src $(IMAGE)

src:	
	make -C src

$(IMAGE): $(BINFILES) $(LIBFILES) $(ETCFILES)
	rm -rf $(STAGEDIR)
	mkdir -p $(STAGEDIR)/bin
	mkdir -p $(STAGEDIR)/lib
	mkdir -p $(STAGEDIR)/etc
	mkdir -p $(STAGEDIR)/.mpd
	cp -a $(BINFILES) $(STAGEDIR)/bin
	cp -a $(LIBFILES) $(STAGEDIR)/lib
	cp -a $(ETCFILES) $(STAGEDIR)/etc
	cp ../../../MPD.txt output/README.txt
	strip $(STAGEDIR)/bin/* || exit 0
	cd output; tar cfz ../$(IMAGE) --owner 8000 --group 8000 .
	
clean:
	rm -rf output $(IMAGE)
