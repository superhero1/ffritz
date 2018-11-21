# Generates application image for atom
#
# Options: 
#	CASH    relative path to cash tool
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

BINFILES    = ../mpd/build/src/mpd src/sndlist src/usbplay src/usbplayd src/ffdaemon
BINFILES   += $(BR_TGT_BIN)/strace $(BR_TGT_BIN)/ldd $(BR_TGT_SBIN)/tcpdump 
BINFILES   += $(BR_TGT_BIN)/ltrace
BINFILES   += $(BR_TGT_BIN)/tcpbridge $(BR_TGT_BIN)/tcpprep $(BR_TGT_BIN)/tcpreplay $(BR_TGT_BIN)/tcpreplay-edit $(BR_TGT_BIN)/tcprewrite
BINFILES   += $(BR_TGT_BIN)/mpc $(BR_TGT_BIN)/rsync $(BR_TGT_BIN)/socat 
BINFILES   += $(BR_TGT)/../host/usr/i686-buildroot-linux-uclibc/sysroot/usr/bin/curl
BINFILES   += $(BR_TGT_SBIN)/openvpn
BINFILES   += $(BR_TGT_BIN)/gdbserver
BINFILES   += $(BR_TGT_BIN)/setpci $(BR_TGT_BIN)/lspci
BINFILES   += $(BR_TGT_SBIN)/snmpd
BINFILES   += ../shairport/build/shairport
ETCFILES    = etc/ffritz-env.sh etc/mpd.conf etc/runmpd etc/ff_atom_startup etc/ffservices etc/run_usbplay etc/run_shairport etc/mkusers
ETCFILES   += etc/run_lircd etc/lirc_options_dfl.conf etc/upmpdcli.conf-dist etc/run_bluetooth
ETCFILES   += etc/run_openvpn etc/usrmount etc/fflogrotate etc/ffshutdown
MODFILES    = lib/modules/cdc-acm.ko

STAGEDIR    = $(TOPDIR)/output/ffritz

VERSION     = $(shell cat version)

IMAGE       = ffritz-app-$(VERSION)-fos7.tar
CSUM	    = ffritz-app-$(VERSION)-fos7.sha256sum

ifeq ($(INSTDIR),)
    INSTDIR=$(STAGEDIR)
endif

SUBDIRS	    = ../buildroot ../libmaru ../shairport ../libmpdclient ../mpc ../lirc ../libupnpp ../jsoncpp ../upmpdcli ../libnfs ../nfs ../mpd ../bt src
INSTDIRS    = ../lirc/install ../libupnpp/install ../upmpdcli/install ../nfs/install ../bt/install ../prtg/install ../athtool/install
SUBDIRS    += ../libdvbfi
INSTDIRS   += ../libdvbfi/install

ifeq ($(CASH),)
CASH=$(shell ls -d ../cash 2>/dev/null)
endif

ifneq ($(CASH),)
SUBDIRS    += $(CASH)
INSTDIRS   += $(CASH)/install
endif

ifeq ($(NOSTRIP),)
NOSTRIP	    = 0
endif

.PHONY:	$(SUBDIRS) $(IMAGE)

all:	$(SUBDIRS) $(IMAGE)

modules:
	mkdir -p lib/modules
	make -C ../avm ff-modules-install DESTDIR=$(TOPDIR)/lib/modules

../upmpdcli:	../libupnpp/install

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
	tar cf $(IMAGE) ffimage.bin $(CSUM) *.md README-dvb.txt fwd uclib-version
#	tar cf $(IMAGE) ffimage.bin ffimage.sha256sum *.md uclib-version
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
	cp -a  $(shell ls  $(BR_TGT_LIB)/lib*so*) $(INSTDIR)/lib
	cp -a  $(BR_TGT_LIB1)/libutil* $(INSTDIR)/lib
	cp $(ETCFILES) $(INSTDIR)/etc
	cp -ar etc/openvpn $(INSTDIR)/etc
	sed -i -e 's@^rundir=.*@rundir=/var/media/ftp/ffritz@' $(INSTDIR)/etc/ff_atom_startup
	make -C ../lirc install DESTDIR=$(INSTDIR)
	make -C ../libupnpp install DESTDIR=$(INSTDIR)
	make -C ../upmpdcli install DESTDIR=$(INSTDIR)
	make -C ../nfs install DESTDIR=$(INSTDIR)
	cp -a bin/ffwrap $(INSTDIR)/bin/exec
	@cd $(INSTDIR)/bin; \
	    for f in `ls | xargs file | grep ELF | grep -v cash_wrap.so | sed -e 's/:.*//'`; do\
		test `basename $$f` != cash -a $(NOSTRIP) -eq 0 && strip $$f;\
		mv $$f exec/$$f;\
		ln -sv exec/ffwrap $$f;\
	    done
	@if [ $(NOSTRIP) -eq 0 ]; then \
	    cd $(INSTDIR)/lib; \
	    for l in `ls | xargs file | grep ELF | sed -e 's/:.*//'`; do\
		strip $$l;\
	    done; fi
	cp -ar $(BR_TGT)/usr/share/terminfo $(INSTDIR)/share
	cp -ar $(BR_TGT)/usr/share/snmp $(INSTDIR)/share
	ls ${BR_TGT}/lib/libuClibc-* | sed -e 's/.*-\([0-9]\).*/\1/' > uclib-version || exit 1
	./mkbblinks bb-apps /usr/bin/busybox-i686 output/ffritz/bin $(SQ_ROOT)/sbin $(SQ_ROOT)/bin $(SQ_ROOT)/usr/sbin $(SQ_ROOT)/usr/bin $(SQ_ROOT)/usr/local/bin
	
clean: 
	rm -rf output $(IMAGE) ffimage.bin

cleanall:
	for d in $(SUBDIRS); do make -C $${d} clean; done
