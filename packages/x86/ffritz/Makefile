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

BINFILES    = ../mpd/build/src/mpd src/sndlist src/usbplay
LIBFILES    = $(shell ls ../buildroot/build/output/target/usr/lib/lib*so*) $(shell ls ../libmaru/lib*so*)
ETCFILES    = etc/ffritz-env.sh etc/mpd.conf etc/runmpd etc/ff_atom_startup

STAGEDIR    = $(TOPDIR)/output/ffritz

VERSION     = $(shell cat version)

IMAGE       = ffritz-x86-$(VERSION).tar.gz

ifeq ($(INSTDIR),)
    INSTDIR=$(STAGEDIR)
endif

SUBDIRS	    = ../buildroot ../mpd ../libmaru ../dropbear src

.PHONY:	$(SUBDIRS)

all:	$(SUBDIRS) $(IMAGE)

$(SUBDIRS):	
	make -C $@

$(IMAGE): $(BINFILES) $(LIBFILES) $(ETCFILES)
	rm -rf $(STAGEDIR)
	make install INSTDIR=$(STAGEDIR)
	cp ../../../MPD.txt output/README.txt
	cd output; tar cfz ../$(IMAGE) --owner 8000 --group 8000 .

# Install averything to target
# LD_LIBRARY_PATH needs to be set correctly for binaries. This is done by the ffwrap wrapper script,
# which is linked to for each binary. This makes the package position independent, i.e.
# it can be installed to /var/media/ftp, or integrated into the atom squashfs (to /usr/local).
#
install: $(SUBDIRS) $(BINFILES) $(LIBFILES) $(ETCFILES)
	mkdir -p $(INSTDIR)/bin/exec
	mkdir -p $(INSTDIR)/lib
	mkdir -p $(INSTDIR)/etc
	cp -a $(BINFILES) $(INSTDIR)/bin
	cp -a $(LIBFILES) $(INSTDIR)/lib
	cp $(ETCFILES) $(INSTDIR)/etc
	sed -i -e 's@^rundir=.*@rundir=/var/media/ftp/ffritz@' $(INSTDIR)/etc/ff_atom_startup
	chmod +x $(INSTDIR)/etc/ff_atom_startup
	make -C ../dropbear install DESTDIR=$(INSTDIR)
	mv $(INSTDIR)/usr/local/bin/* $(INSTDIR)/bin
	mv $(INSTDIR)/usr/local/sbin/* $(INSTDIR)/bin
	cd $(INSTDIR)/bin; ln -s dbclient ssh
	cp -a bin/ffwrap $(INSTDIR)/bin/exec
	cd $(INSTDIR)/bin; \
	    for f in `ls | xargs file | grep ELF | sed -e 's/:.*//'`; do\
		strip $$f;\
		mv -v $$f exec/$$f;\
		ln -sv exec/ffwrap $$f;\
	    done
	rm -rf $(INSTDIR)/usr
	
clean:
	rm -rf output $(IMAGE)

cleanall:
	for d in $(SUBDIRS); do make -C $${d} clean; done
