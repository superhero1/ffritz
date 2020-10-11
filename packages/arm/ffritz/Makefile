# Generates application image for atom
#
# Options: 
#	CASH    relative path to cash tool
#	FFAD    relative path to ffad tool
#       NOSTRIP Set to 1 to not strip binaries (for debugging)
#
ARCHDIR=$(shell while test -f arch.mk && echo $$PWD && exit 0; test $$PWD != /; do cd ..; done)
include $(ARCHDIR)/arch.mk


BR_TGT	    = $(realpath $(BUILDROOT)/output/target)
BR_TGT_LIB1 = $(BR_TGT)/lib
BR_TGT_LIB  = $(BR_TGT)/usr/lib
BR_TGT_BIN  = $(BR_TGT)/usr/bin
BR_TGT_SBIN = $(BR_TGT)/usr/sbin

BINFILES   += $(wildcard bin/*)
BINFILES   += $(wildcard bin$(BR_VERSION)/*)
BINFILES   += $(wildcard $(CMNDIR)/ffritz/bin/*)
BINFILES   += $(wildcard $(CMNDIR)/ffritz/bin$(BR_VERSION)/*)

OUTPUTDIR   = $(TOPDIR)/output$(BR_VERSION)
STAGEDIR    = $(OUTPUTDIR)/ffritz-arm

BRCOPY       = $(STAGEDIR)/buildroot

VERSION     = $(shell cat version)

IMAGE       = $(ARM_EXT_IMAGE)

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
	make -C $(BRBUILDDIR)

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
	mkdir -p $(DLDIR)
	make -C $@

$(INSTDIRS):
	make -C `dirname $@` DESTDIR=$(STAGEDIR) INSTDIR=$(STAGEDIR)
	make -C `dirname $@` install DESTDIR=$(STAGEDIR) INSTDIR=$(STAGEDIR)

$(IMAGE): $(BINFILES) $(ETCFILES) stage
	cd $(OUTPUTDIR); tar cfz ../$(IMAGE) .
	@echo
	@echo +++ DONE +++
	@echo "ARM image file: $(IMAGE)"
	@echo


# Install everything to target
# LD_LIBRARY_PATH needs to be set correctly for binaries. This is done by the ffwrap wrapper script,
# which is linked to for each binary. This makes the package position independent, i.e.
# it can be installed to /var/media/ftp, or integrated into the atom squashfs (to /usr/local).
#
stage: $(BINFILES) $(ETCFILES) $(INSTDIRS)
	cp -a $(BINFILES) $(INSTDIR)/bin
	mv -v $(INSTDIR)/usr/bin/* $(INSTDIR)/usr/sbin/* $(INSTDIR)/bin
	cd $(BRCOPY)/usr/lib; ln -s ../../lib/libutil* . || true
	mv $(INSTDIR)/bin/ffwrap $(BRCOPY)/usr/bin
	cp $(BRCOPY)/usr/bin/ffwrap $(BRCOPY)/usr/sbin
	# Move our binaries to buildroot
	@cd $(INSTDIR)/bin; \
	    for f in `ls | xargs file | grep ELF | grep -v cash_wrap.so | sed -e 's/:.*//'`; do\
		test `basename $$f` != cash -a $(NOSTRIP) -eq 0 && strip $$f;\
		mv $$f $(BRCOPY)/usr/bin/$$f;\
	    done
	# Make buildroot usr/bin available for host
	@cd $(BRCOPY)/usr/bin; \
	    for f in `ls | xargs file -L | grep ELF | grep -v cash_wrap.so | sed -e 's/:.*//'`; do\
		test -e ../../../bin/$$f || ln -sv ../buildroot/usr/bin/ffwrap ../../../bin/$$f;\
	    done
	# /usr/bin non-ELF
	@cd $(BRCOPY)/usr/bin; \
	    for f in `ls | xargs file -L | grep -v ELF | sed -e 's/:.*//'`; do\
		test -e ../../../bin/$$f || ln -sv ../buildroot/usr/bin/$$f ../../../bin/$$f;\
	    done
	# Make buildroot usr/sbin available for host
	@cd $(BRCOPY)/usr/sbin; \
	    for f in `ls | xargs file -L | grep ELF | sed -e 's/:.*//'`; do\
		test -e ../../../bin/$$f || ln -sv ../buildroot/usr/sbin/ffwrap ../../../bin/$$f;\
	    done
	# /usr/sbin non-ELF
	@cd $(BRCOPY)/usr/sbin; \
	    for f in `ls | xargs file -L | grep -v ELF | sed -e 's/:.*//'`; do\
		test -e ../../../bin/$$f || ln -sv ../buildroot/usr/sbin/$$f ../../../bin/$$f;\
	    done
	cd $(INSTDIR)/share; ln -s ../buildroot/usr/share/terminfo .
	readlink ${BR_TGT}/lib/libc.so.* | xargs basename > glibc-version || exit 1
	
clean: 
	rm -rf $(OUTPUTDIR) $(IMAGE) ffimage.bin
	for d in $(SUBDIRS) $(INSTDIRS:%/install=%); do make -k -C $$d clean || true; done

cleanall: clean
	make -C $(BUILDROOT) clean
