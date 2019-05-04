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

BR_OUTPUT   = $(PKGDIR)/buildroot/build/output

BR_TGT	    = $(BR_OUTPUT)/target
BR_TGT_LIB  = $(BR_TGT)/usr/lib
BR_TGT_BIN  = $(BR_TGT)/usr/bin
BR_TGT_SBIN = $(BR_TGT)/usr/sbin
BR_HOST	    =  $(BR_OUTPUT)/host
BR_HOST_BIN =  $(BR_HOST)/usr/armeb-buildroot-linux-uclibcgnueabi/bin/

STAGEDIR    = $(TOPDIR)/output/ffritz-arm

VERSION     = $(shell cat version)

IMAGE       = ffritz-arm-$(VERSION)-fos7.tar.gz

ifeq ($(INSTDIR),)
    INSTDIR=$(STAGEDIR)
endif

SUBDIRS	    = ../buildroot 

.PHONY:	$(SUBDIRS) output

all:	$(SUBDIRS) $(IMAGE)

$(SUBDIRS):	
	make -C $@

$(IMAGE): output
	rm -rf $(STAGEDIR)
	make install INSTDIR=$(STAGEDIR)
	cp ../../../README.md output/
	cd output; tar cfz ../$(IMAGE) --owner=8000 --group=8000 .
	cd output; tar cfz ../bin-$(IMAGE) --owner=8000 --group=8000 ./ffritz-arm/bin ./ffritz-arm/etc
	@echo
	@echo +++ DONE +++
	@echo "ARM image file: $(TOPDIR)/$(IMAGE)"
	@echo

# Install averything to target
# LD_LIBRARY_PATH needs to be set correctly for binaries. This is done by the ffwrap wrapper script,
# which is linked to for each binary. This makes the package position independent, i.e.
# it can be installed to /var/media/ftp, or integrated into the atom squashfs (to /usr/local).
#
install:
	mkdir -p $(INSTDIR)/bin/exec
	mkdir -p $(INSTDIR)/lib
	mkdir -p $(INSTDIR)/etc
	cp -a `find $(BR_TGT_BIN) -type f` $(INSTDIR)/bin
	cp -a `find $(BR_TGT_SBIN) -type f` $(INSTDIR)/bin
	cp -a $(BR_TGT_LIB)/* $(INSTDIR)/lib
	cp -a bin/ffwrap $(INSTDIR)/bin/exec
	cd $(INSTDIR)/bin; \
	    for f in `ls | xargs file | grep ELF | sed -e 's/:.*//'`; do\
		$(BR_HOST_BIN)/strip $$f;\
		mv -v $$f exec/$$f;\
		ln -sv exec/ffwrap $$f;\
	    done
	rm -rf $(INSTDIR)/usr
	
clean:
	rm -rf output $(IMAGE)

cleanall:
	for d in $(SUBDIRS); do make -C $${d} clean; done
