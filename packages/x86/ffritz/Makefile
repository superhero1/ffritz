# This makefile either generates a tarball which is supposed to
# ccontain binaries run in /var/media/ftp/ffritz, or
# copies files into a root fs image where they are installed
# to and executed from /usr/local.
#
# For the first, just call make
# For the latter, call make install INSTDIR=<install-path>
#   where install-path is the absolute path where bin, lib and etc shall
#   reside (etc. $squashfs-root/usr/local)
#
#
TOPDIR	    = $(shell pwd)
PKGDIR	    = $(shell cd ..; pwd)

BR_TGT	    = ../buildroot/build/output/target
BR_TGT_LIB  = $(BR_TGT)/usr/lib
BR_TGT_BIN  = $(BR_TGT)/usr/bin
BR_TGT_SBIN = $(BR_TGT)/usr/sbin

BINFILES    = ../mpd/build/src/mpd src/sndlist src/usbplay src/usbplayd src/su
BINFILES   += $(BR_TGT_BIN)/strace $(BR_TGT_BIN)/ldd $(BR_TGT_SBIN)/tcpdump 
BINFILES   += $(BR_TGT_BIN)/tcpbridge $(BR_TGT_BIN)/tcpprep $(BR_TGT_BIN)/tcpreplay $(BR_TGT_BIN)/tcpreplay-edit $(BR_TGT_BIN)/tcprewrite
BINFILES   += $(BR_TGT_BIN)/mpc $(BR_TGT_BIN)/rsync $(BR_TGT_BIN)/socat
BINFILES   += $(BR_TGT)/../host/usr/i686-buildroot-linux-uclibc/sysroot/usr/bin/curl
BINFILES   += ../shairport/build/shairport
ETCFILES    = etc/ffritz-env.sh etc/mpd.conf etc/runmpd etc/mpdloop etc/ff_atom_startup etc/mkusers etc/run_usbplay etc/run_shairport etc/usrmount

STAGEDIR    = $(TOPDIR)/output/ffritz

VERSION     = $(shell cat ../../../version)

IMAGE       = ffritz-x86-$(VERSION).tar.gz

ifeq ($(INSTDIR),)
    INSTDIR=$(STAGEDIR)
endif

SUBDIRS	    = ../buildroot ../mpd ../libmaru ../dropbear ../shairport ../libmpdclient ../mpc ../lirc src

.PHONY:	$(SUBDIRS)

all:	$(SUBDIRS) $(IMAGE)

$(SUBDIRS):	
	mkdir -p ../../dl
	make -C $@

$(IMAGE): $(BINFILES) $(ETCFILES) $(shell test -d $(BR_TGT_LIB) && ls $(BR_TGT_LIB)/lib*so*)
	rm -rf $(STAGEDIR)
	make install INSTDIR=$(STAGEDIR)
	cp ../../../MPD.md output/
	cp ../../../README.md output/
	cd output; tar cfz ../$(IMAGE) --owner=8000 --group=8000 .
	cd output; tar cfz ../bin-$(IMAGE) --owner=8000 --group=8000 ./ffritz/bin ./ffritz/etc

# Install averything to target
# LD_LIBRARY_PATH needs to be set correctly for binaries. This is done by the ffwrap wrapper script,
# which is linked to for each binary. This makes the package position independent, i.e.
# it can be installed to /var/media/ftp, or integrated into the atom squashfs (to /usr/local).
#
install: $(SUBDIRS) $(BINFILES) $(ETCFILES) $(shell test -d $(BR_TGT_LIB) && ls $(BR_TGT_LIB)/lib*so*)
	mkdir -p $(INSTDIR)/bin/exec
	mkdir -p $(INSTDIR)/lib
	mkdir -p $(INSTDIR)/etc
	cp -a $(BINFILES) $(INSTDIR)/bin
	cp -a  $(shell ls  $(BR_TGT_LIB)/lib*so*) $(INSTDIR)/lib
	cp $(ETCFILES) $(INSTDIR)/etc
	sed -i -e 's@^rundir=.*@rundir=/var/media/ftp/ffritz@' $(INSTDIR)/etc/ff_atom_startup
	chmod +x $(INSTDIR)/etc/ff_atom_startup
	make -C ../dropbear install DESTDIR=$(INSTDIR)
	make -C ../lirc install DESTDIR=$(INSTDIR)
	mv $(INSTDIR)/usr/local/bin/* $(INSTDIR)/bin
	mv $(INSTDIR)/usr/local/sbin/* $(INSTDIR)/bin
	cp -a bin/ffwrap $(INSTDIR)/bin/exec
	cd $(INSTDIR)/bin; \
	    for f in `ls | xargs file | grep ELF | sed -e 's/:.*//'`; do\
		strip $$f;\
		mv -v $$f exec/$$f;\
		ln -sv exec/ffwrap $$f;\
	    done
	cd $(INSTDIR)/bin; ln -s exec/ffwrap ssh
	cd $(INSTDIR)/bin/exec; ln -s dbclient ssh
	rm -rf $(INSTDIR)/usr
	
clean: 
	rm -rf output $(IMAGE)

cleanall:
	for d in $(SUBDIRS); do make -C $${d} clean; done
