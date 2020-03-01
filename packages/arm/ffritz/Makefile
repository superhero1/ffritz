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

BR_TGT	    = $(shell readlink -f ../buildroot/build/output/target)
BR_TGT_LIB1 = $(BR_TGT)/lib
BR_TGT_LIB  = $(BR_TGT)/usr/lib
BR_TGT_BIN  = $(BR_TGT)/usr/bin
BR_TGT_SBIN = $(BR_TGT)/usr/sbin

#BINFILES    = src/sndlist src/usbplay src/usbplayd src/ffdaemon
#BINFILES   += ../shairport/build/shairport
BINFILES   += $(wildcard bin/*)
#ETCFILES    = etc/ffritz-env.sh etc/mpd.conf etc/ff_atom_startup etc/ffservices etc/mkusers
#ETCFILES   += etc/lirc_options_dfl.conf
#ETCFILES   += etc/upmpdcli.conf-dist
#ETCFILES   += etc/usrmount etc/fflogrotate etc/ffshutdown
#ETCFILES   += etc/init.d
#ETCFILES   += etc/shairport-sync.conf.dfl
#ETCFILES   += etc/exports

STAGEDIR    = $(TOPDIR)/output/ffritz-arm

BRCOPY       = $(STAGEDIR)/buildroot

VERSION     = $(shell cat version)

IMAGE       = $(TOPDIR)/ffritz-arm-$(VERSION)-fos7.tar.gz

ifeq ($(INSTDIR),)
    INSTDIR=$(STAGEDIR)
endif

## Packages
#

## Packages with (build/)install target
#
INSTDIRS   += ../dropbear/install

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

.PHONY:	$(SUBDIRS) $(INSTDIRS) $(IMAGE) $(INSTDIR)

all:	buildroot $(INSTDIR) $(SUBDIRS) $(IMAGE)

buildroot:
	make -C ../buildroot

$(INSTDIR):
	mkdir -p $(INSTDIR)
	rm -rf $(INSTDIR)/*
	mkdir -p $(INSTDIR)/etc
	mkdir -p $(INSTDIR)/bin
	mkdir -p $(INSTDIR)/share
	mkdir -p $(BRCOPY)
	cd $(BRCOPY); tar xf $(BR_TGT)/../images/rootfs.tar 
	cd $(INSTDIR); ln -s buildroot/usr/lib .

$(SUBDIRS):	
	mkdir -p ../../dl
	make -C $@

$(INSTDIRS):
	make -C `dirname $@` install DESTDIR=$(STAGEDIR) INSTDIR=$(STAGEDIR)

$(IMAGE): $(BINFILES) $(ETCFILES) stage
	cd output; tar cfz $(IMAGE) .
	@echo
	@echo +++ DONE +++
	@echo "ARM image file: $(IMAGE)"
	@echo

$(IMAGE)XX: ffimage.bin
	sha256sum ffimage.bin | sed -e 's/ .*//' > $(CSUM)
	cp ../../../*.md .
#	cp ../libdvbfi/README.txt README-dvb.txt 
#	cp ../libdvbfi/fwd .
	cp $(CSUM) ffimage.sha256sum
	tar cf $(IMAGE) ffimage.bin ffimage.sha256sum *.md glibc-version
	rm -f ffimage.sha256sum
	@echo
	@echo +++ DONE +++
	@echo "ARM image file: $(TOPDIR)/$(IMAGE)"
	@echo "Image checksum:  `cat $(CSUM)`"
	@echo

ffimage.bin: $(BINFILES) $(ETCFILES) stage
	rm -f $@
	cd output; mksquashfs ffritz ../$@ -all-root -no-progress -no-exports -no-sparse -b 65536 

# Install everything to target
# LD_LIBRARY_PATH needs to be set correctly for binaries. This is done by the ffwrap wrapper script,
# which is linked to for each binary. This makes the package position independent, i.e.
# it can be installed to /var/media/ftp, or integrated into the atom squashfs (to /usr/local).
#
stage: $(BINFILES) $(ETCFILES) $(INSTDIRS)
	cp -a $(BINFILES) $(INSTDIR)/bin
	mv $(INSTDIR)/usr/bin/* $(INSTDIR)/usr/sbin/* $(INSTDIR)/bin
#	cp -r $(ETCFILES) $(INSTDIR)/etc
#	cp -ar etc/openvpn $(INSTDIR)/etc
#	sed -i -e 's@^rundir=.*@rundir=/var/media/ftp/ffritz@' $(INSTDIR)/etc/ff_atom_startup
	cd $(BRCOPY)/usr/lib; ln -s ../../lib/libutil* . || true
	mv $(INSTDIR)/bin/ffwrap $(BRCOPY)/usr/bin
	# Move our binaries to buildroot
	@cd $(INSTDIR)/bin; \
	    for f in `ls | xargs file | grep ELF | grep -v cash_wrap.so | sed -e 's/:.*//'`; do\
		test `basename $$f` != cash -a $(NOSTRIP) -eq 0 && strip $$f;\
		mv $$f $(BRCOPY)/usr/bin/$$f;\
	    done
	# Make buldroot usr/(s)bin available for host
	@cd $(BRCOPY)/usr/bin; \
	    for f in `ls | xargs file -L | grep ELF | grep -v cash_wrap.so | sed -e 's/:.*//'`; do\
		ln -sv ../buildroot/usr/bin/ffwrap ../../../bin/$$f;\
	    done
	@cd $(BRCOPY)/usr/sbin; \
	    cp ../bin/ffwrap .; \
	    for f in `ls | xargs file -L | grep ELF | sed -e 's/:.*//'`; do\
		ln -sv ../buildroot/usr/sbin/ffwrap ../../../bin/$$f;\
	    done
	# strip libraries
#	@if [ $(NOSTRIP) -eq 0 ]; then \
#	    cd $(INSTDIR)/lib; \
#	    for l in `ls | xargs file | grep ELF | sed -e 's/:.*//'`; do\
#		strip $$l;\
#	    done; fi
	# shairport does not like those
#	rm -f $(INSTDIR)/bin/avahi*
	cd $(INSTDIR)/share; ln -s ../buildroot/usr/share/terminfo .
	# buildroot binaries link against /lib/libc.so.0
#	cd $(BRCOPY)/usr/lib; ln -s ../../lib/libuClibc-1.0.*.so libc.so.0; ln -s ../../lib/libuClibc-1.0.*.so libc.so.1
	readlink ${BR_TGT}/lib/libc.so.* | xargs basename > glibc-version || exit 1
	
clean: 
	rm -rf output $(IMAGE) ffimage.bin
	for d in $(SUBDIRS) $(INSTDIRS:%/install=%); do make -k -C $$d clean || true; done

cleanall: clean
	make -C ../buildroot clean
