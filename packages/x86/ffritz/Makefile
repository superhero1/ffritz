# Generates application image for atom
#
# Options: 
#	CASH    relative path to cash tool
#	FFAD    relative path to ffad tool
#       NOSTRIP Set to 1 to not strip binaries (for debugging)
#
TOPDIR	    = $(shell pwd)
PKGDIR	    = $(shell cd ..; pwd)

SQ_ROOT     = ../../../atom/squashfs-root

BR_TGT	    = ../buildroot/build/output/target
BR_TGT_LIB1 = $(BR_TGT)/lib
BR_TGT_LIB  = $(BR_TGT)/usr/lib
BR_TGT_BIN  = $(BR_TGT)/usr/bin
BR_TGT_SBIN = $(BR_TGT)/usr/sbin

BINFILES    = src/sndlist src/usbplay src/usbplayd src/ffdaemon
BINFILES   += ../shairport/build/shairport
BINFILES   += $(wildcard bin/*)
ETCFILES    = etc/ffritz-env.sh etc/mpd.conf etc/ff_atom_startup etc/ffservices etc/mkusers
ETCFILES   += etc/lirc_options_dfl.conf etc/upmpdcli.conf-dist
ETCFILES   += etc/usrmount etc/fflogrotate etc/ffshutdown
ETCFILES   += etc/init.d
MODFILES    = lib/modules/cdc-acm.ko

STAGEDIR    = $(TOPDIR)/output/ffritz

VERSION     = $(shell cat version)

IMAGE       = ffritz-app-$(VERSION)-fos7.tar
CSUM	    = ffritz-app-$(VERSION)-fos7.sha256sum

ifeq ($(INSTDIR),)
    INSTDIR=$(STAGEDIR)
endif

## Packages
#
SUBDIRS	    = ../buildroot
SUBDIRS    += ../libmaru
SUBDIRS    += ../shairport 
SUBDIRS    += ../libmpdclient 
SUBDIRS    += ../lirc 
SUBDIRS    += ../jsoncpp 
SUBDIRS    += ../libnfs 
SUBDIRS    += ../nfs 
SUBDIRS    += src

## Packages with (build/)install target
#
INSTDIRS   += ../lirc/install 
INSTDIRS   += ../upmpdcli/install 
INSTDIRS   += ../nfs/install 
INSTDIRS   += ../bt/install 
INSTDIRS   += ../prtg/install 
INSTDIRS   += ../athtool/install 
INSTDIRS   += ../unionfs-fuse/install 
INSTDIRS   += ../dnsmasq/install
INSTDIRS   += ../libdvbfi/install
INSTDIRS   += ../mpc/install
INSTDIRS   += ../mpd/install

ifeq ($(CASH),)
CASH=$(shell ls -d ../cash/build/.. 2>/dev/null)
endif

ifneq ($(CASH),)
SUBDIRS    += $(CASH)
INSTDIRS   += $(CASH)/install
endif

ifeq ($(FFAD),)
FFAD=$(shell ls -d ../ffad/build/.. 2>/dev/null)
endif

ifneq ($(FFAD),)
SUBDIRS    += $(FFAD)
INSTDIRS   += $(FFAD)/install
endif

ifeq ($(NOSTRIP),)
NOSTRIP	    = 0
endif

.PHONY:	$(SUBDIRS) $(INSTDIRS) $(IMAGE)

all:	$(SUBDIRS) $(IMAGE)

modules:
	mkdir -p lib/modules
	make -C ../avm ff-modules-install DESTDIR=$(TOPDIR)/lib/modules

$(SUBDIRS):	
	mkdir -p ../../dl
	make -C $@

$(INSTDIRS):
	make -C `dirname $@` install DESTDIR=$(INSTDIR)

$(IMAGE): ffimage.bin
	sha256sum ffimage.bin | sed -e 's/ .*//' > $(CSUM)
	cp ../../../*.md .
	cp ../libdvbfi/README.txt README-dvb.txt 
	cp ../libdvbfi/fwd .
	cp $(CSUM) ffimage.sha256sum
	tar cf $(IMAGE) ffimage.bin ffimage.sha256sum *.md uclib-version
	rm -f ffimage.sha256sum
	@echo
	@echo +++ DONE +++
	@echo "Atom image file: $(TOPDIR)/$(IMAGE)"
	@echo "Image checksum:  `cat $(CSUM)`"
	@echo
	

ffimage.bin: $(BINFILES) $(ETCFILES) $(shell test -d $(BR_TGT_LIB) && ls $(BR_TGT_LIB)/lib*so*)
	rm -rf $(STAGEDIR)
	make install INSTDIR=$(STAGEDIR)
	rm -f $@
	cd output; mksquashfs ffritz ../$@ -all-root -no-progress -no-exports -no-sparse -b 65536 

# Install everything to target
# LD_LIBRARY_PATH needs to be set correctly for binaries. This is done by the ffwrap wrapper script,
# which is linked to for each binary. This makes the package position independent, i.e.
# it can be installed to /var/media/ftp, or integrated into the atom squashfs (to /usr/local).
#
install: $(BINFILES) $(ETCFILES) $(INSTDIRS) modules $(shell test -d $(BR_TGT_LIB) && ls $(BR_TGT_LIB)/lib*so*)
	mkdir -p $(INSTDIR)/bin/exec
	mkdir -p $(INSTDIR)/lib/modules
	mkdir -p $(INSTDIR)/etc
	cp -a $(BINFILES) $(INSTDIR)/bin
	cp -a $(MODFILES) $(INSTDIR)/lib/modules
	cp -r $(ETCFILES) $(INSTDIR)/etc
	cp -ar etc/openvpn $(INSTDIR)/etc
	sed -i -e 's@^rundir=.*@rundir=/var/media/ftp/ffritz@' $(INSTDIR)/etc/ff_atom_startup
	rsync -a $(BR_TGT)/ $(INSTDIR)/buildroot 
	cd $(INSTDIR)/bin/exec; ln -s ../../buildroot/usr/bin/* . || true
	cd $(INSTDIR)/lib; ln -s ../buildroot/usr/lib/* . || true
	cd $(INSTDIR)/lib; ln -s ../buildroot/lib/libutil* . || true
	mv $(INSTDIR)/bin/ffwrap $(INSTDIR)/bin/exec
#	test -d $(INSTDIR)/sbin && mv $(INSTDIR)/sbin/* $(INSTDIR)/bin
	@cd $(INSTDIR)/bin; \
	    for f in `ls | xargs file | grep ELF | grep -v cash_wrap.so | sed -e 's/:.*//'`; do\
		test `basename $$f` != cash -a $(NOSTRIP) -eq 0 && strip $$f;\
		mv $$f exec/$$f;\
	    done
	@cd $(INSTDIR)/bin/exec; \
	    for f in `ls | xargs file -L | grep ELF | grep -v cash_wrap.so | sed -e 's/:.*//'`; do\
		ln -sv exec/ffwrap ../$$f;\
	    done
	@if [ $(NOSTRIP) -eq 0 ]; then \
	    cd $(INSTDIR)/lib; \
	    for l in `ls | xargs file | grep ELF | sed -e 's/:.*//'`; do\
		strip $$l;\
	    done; fi
	rm -f $(INSTDIR)/bin/avahi*
	cp -ar $(BR_TGT)/usr/share/terminfo $(INSTDIR)/share
	cp -ar $(BR_TGT)/usr/share/snmp $(INSTDIR)/share
	cp -ar $(BR_TGT)/usr/share/nmap $(INSTDIR)/share
	cd $(INSTDIR)/bin/exec; ln -s ../../share/nmap/* .
	ls ${BR_TGT}/lib/libuClibc-* | sed -e 's/.*-\([0-9]\).*/\1/' > uclib-version || exit 1
#	./mkbblinks bb-apps /usr/bin/busybox-i686 output/ffritz/bin $(SQ_ROOT)/sbin $(SQ_ROOT)/bin $(SQ_ROOT)/usr/sbin $(SQ_ROOT)/usr/bin $(SQ_ROOT)/usr/local/bin
	
clean: 
	rm -rf output $(IMAGE) ffimage.bin
	for d in $(SUBDIRS) $(INSTDIRS:%/install=%); do if [ $$d != ../buildroot ]; then make -k -C $$d clean || true; fi; done

cleanall:
	for d in $(SUBDIRS) $(INSTDIRS:%/install=%); do make -k -C $${d} clean || true; done
