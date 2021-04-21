# Generates application image for atom
#
# Options: conf.mk
#
ARCHDIR := $(shell while test -f arch.mk && echo $$PWD && exit 0; test $$PWD != /; do cd ..; done)
include $(ARCHDIR)/arch.mk

BR_TGT	    = $(BUILDROOT)/output/target
BR_TGT_LIB1 = $(BR_TGT)/lib
BR_TGT_LIB  = $(BR_TGT)/usr/lib
BR_TGT_BIN  = $(BR_TGT)/usr/bin
BR_TGT_SBIN = $(BR_TGT)/usr/sbin

BINFILES    += $(wildcard bin/*)
BINFILES    += $(wildcard $(CMNDIR)/ffritz/bin/*)
BINFILES    += $(wildcard $(CMNDIR)/ffritz/bin$(BR_VERSION)/*)
ETCFILES    := etc/ffritz-env.sh etc/mpd.conf etc/ff_atom_startup etc/ffservices etc/mkusers
ETCFILES    += etc/ffwenv
ETCFILES    += etc/lirc_options_dfl.conf
ETCFILES    += etc/upmpdcli.conf-dist
ETCFILES    += etc/usrmount etc/fflogrotate etc/ffshutdown
ETCFILES    += etc/init.d
ETCFILES    += $(wildcard etc/shairport-*)
ETCFILES    += $(wildcard etc/pihole-*)
ETCFILES    += etc/exports

OUTPUTDIR   = $(TOPDIR)/output$(BR_VERSION)
STAGEDIR    = $(OUTPUTDIR)/ffritz
STAGEDIR2   = $(OUTPUTDIR)/image
BRCOPY      = $(STAGEDIR)/buildroot

VERSION     := $(shell cat version)
FFLIB_SRC   := $(shell ls lib/fflibs$(BR_VERSION) 2>/dev/null || ls lib/fflibs)

SQIMAGE     := $(STAGEDIR2)/ffimage.bin
CSUM	    = $(STAGEDIR2)/$(shell basename $(APP_IMAGE) .tar).sha256sum

IMAGE       := $(APP_IMAGE)

-include conf.mk

ifeq ($(MODULES),)
MODULES   += nfs
MODULES   += bt
MODULES   += prtg
MODULES   += modules
MODULES   += wireguard-tools
MODULES   += unfs
MODULES   += cash
MODULES   += ffad
ifneq ($(BR_VERSION),-2019.05)
MODULES   += ftl
MODULES   += libmultid
MODULES   += pi-hole
endif
endif

# required modules
MODULES   += usbplayd
MODULES   += tools

MODULES_INST = $(MODULES:%=%-install)
MODULES_CLEAN = $(MODULES:%=%-clean)

BR_IMAGE = $(BR_TGT)/../images/rootfs.tar

.PHONY:	$(MODULES) $(MODULES_INST) $(IMAGE) $(STAGEDIR)


all:	toolchain $(BR_IMAGE) $(MODULES) $(IMAGE)

submodules: $(MODULES)

submodules-install: $(MODULES_INST)

#$(BR_IMAGE): buildroot

toolchain: buildroot

buildroot:
	@echo $(BRBUILDDIR): build
	@$(SUBMAKE) $(BRBUILDDIR)
	@echo $(BRBUILDDIR): done

# create/empty staging dir, copy buildroot and required files
stage1: $(BR_IMAGE) $(ETCFILES) $(BINFILES)
	echo +++ { stage1 start
	$(call RMDIR,$(STAGEDIR))
	mkdir -p $(STAGEDIR)
	$(call RMDIR,$(STAGEDIR2))
	mkdir -p $(STAGEDIR2)
	mkdir -p $(STAGEDIR)/etc
	mkdir -p $(STAGEDIR)/bin
	mkdir -p $(STAGEDIR)/share
	mkdir -p $(BRCOPY)
	cd $(BRCOPY); tar xf $(BR_IMAGE)
	cd $(STAGEDIR); $(SYMLINK) buildroot/usr/lib .
	cp -r $(BINFILES) $(STAGEDIR)/bin
	cp -r $(ETCFILES) $(STAGEDIR)/etc
	cp -ar etc/openvpn $(STAGEDIR)/etc
	echo +++ } stage1 end

$(MODULES):
	@echo +++ $@: build
	@$(SUBMAKE) ../$@

$(MODULES_INST):
	@echo +++ $(@:%-install=%): install to staging
	@$(SUBMAKE) ../$(@:%-install=%) install DESTDIR=$(BRCOPY)/usr INSTDIR=$(BRCOPY)/usr

$(MODULES_CLEAN):
	@echo +++ $(@:%-clean=%): clean
	@$(SUBMAKEK) ../$(@:%-clean=%) clean || true

$(IMAGE): $(SQIMAGE)
	@cp ../../../*.md $(STAGEDIR2)
	@cp $(CSUM) $(TOPDIR)
	@cp $(CSUM) $(STAGEDIR2)/ffimage.sha256sum
	@cd $(STAGEDIR2); tar cf $(TOPDIR)/$(IMAGE) *
	@echo
	@echo +++ DONE +++
	@echo "Atom image file: $(TOPDIR)/$(IMAGE)"
	@echo "Image checksum:  `cat $(CSUM)`"
	@echo

image: $(IMAGE)

# Finalize staging directory after having created it and modules have installed into it
# LD_LIBRARY_PATH needs to be set correctly for binaries. This is done by
# ffwrap, which is linked to for each binary.
# This makes the package position independent, i.e. it can be installed to
# /var/media/ftp, or integrated into the atom squashfs (to /usr/local).
# LD_LIBRARY path settings specific to the OS/tool are stored in 
# bin/.fflibs, which is read by ffwrap before calling the executable.
#
stage2: stage1 $(MODULES_INST)
	@echo +++ { stage2 start
	@sed -i -e 's@^rundir=.*@rundir=/var/media/ftp/ffritz@' $(STAGEDIR)/etc/ff_atom_startup
	@cd $(BRCOPY)/usr/lib; $(SYMLINK) ../../lib/libutil* . || true
	@cp -v $(BRCOPY)/usr/bin/ffwrap $(BRCOPY)/usr/sbin
	@mv $(BRCOPY)/usr/bin/ffdaemon $(STAGEDIR)/bin
	# Make buildroot usr/bin, sbin available for host
	@$(call LINK_TO_ONE,$(call LS_ELF,$(BRCOPY)/usr/bin),$(STAGEDIR)/bin/,../buildroot/usr/bin/ffwrap)
	@$(call LINK_TO_ONE,$(call LS_ELF,$(BRCOPY)/usr/sbin),$(STAGEDIR)/bin/,../buildroot/usr/sbin/ffwrap)
	# /usr/bin, sbin non-ELF
	@$(call LINK_TO_DIR,$(call LS_NOT_ELF,$(BRCOPY)/usr/bin),$(STAGEDIR)/bin/,../buildroot/usr/bin/)
	@$(call LINK_TO_DIR,$(call LS_NOT_ELF,$(BRCOPY)/usr/sbin),$(STAGEDIR)/bin/,../buildroot/usr/sbin/)
	ls -lL $(STAGEDIR)/bin >/dev/null
# shairport does not like those
	rm -f $(STAGEDIR)/bin/avahi*
# Provide terminfo database
	cd $(STAGEDIR)/share; $(SYMLINK) ../buildroot/usr/share/terminfo .
	cp $(FFLIB_SRC) $(BRCOPY)/usr/bin/.fflibs
	cp $(FFLIB_SRC) $(BRCOPY)/usr/sbin/.fflibs
	@echo +++ } stage2 end

$(SQIMAGE): stage2
	@echo +++ { build overlay image
	@readlink ${BR_TGT}/lib/libc.so.* | xargs basename > $(STAGEDIR2)/glibc-version || exit 1
	@rm -f $@
	@cd $(OUTPUTDIR); mksquashfs ffritz $@ -all-root -no-progress -no-exports -no-sparse -b 65536 
	sha256sum $(SQIMAGE) | sed -e 's/ .*//' > $(CSUM)
	@echo +++ } end build overlay image

sqimage: $(SQIMAGE)

clean:	$(MODULES_CLEAN)
	$(call RMDIR,$(OUTPUTDIR))
	$(call RMDIR,/tmp/xxx)

cleanall: clean
	@$(SUBMAKE) $(BRBUILDDIR) clean
