# Default make for atom
#
# Build ffritz package, which builds other packages on demand
#
ARCHDIR=$(shell while test -f arch.mk && echo $$PWD && exit 0; test $$PWD != /; do cd ..; done)
include $(ARCHDIR)/../paths.mk

MODDIR		= $(shell readlink -f ../../atom/mod)
STAGEDIR	= $(shell readlink -f .)/staging

BASE_PACKAGES=$(BRBASEDIR) dropbear privatekeypassword #unionfs-fuse

.PHONY:	$(BASE_PACKAGES)

all:
	make -C ffritz

base:	$(BASE_PACKAGES)

FFLIB	= $(STAGEDIR)/usr/lib/ff/lib
FFEXEC	= $(STAGEDIR)/usr/lib/ff/exec
FFBIN	= $(STAGEDIR)/usr/bin

base-install: stage
	cp -arf $(STAGEDIR)/* $(MODDIR)

stage:
	rm -rf $(STAGEDIR)
	mkdir -p $(STAGEDIR)/lib
	mkdir -p $(FFLIB)
	mkdir -p $(FFEXEC)
	mkdir -p $(FFBIN)
	mkdir -p $(STAGEDIR)/var/empty
	make -e -C $(BUILDROOT)/output/build/libopenssl* install INSTALL_PREFIX=$(STAGEDIR) DESTDIR=$(STAGEDIR)
	rm -f  $(STAGEDIR)/usr/bin/c_rehash $(STAGEDIR)/etc/ssl/openssl.cnf
	rm -f  $(STAGEDIR)/usr/lib/*.a
	rm -rf  $(STAGEDIR)/etc/ssl/man
	make -C privatekeypassword install DESTDIR=$(STAGEDIR)/usr
	make -C dropbear install DESTDIR=$(STAGEDIR) prefix=/usr
	mv $(STAGEDIR)/usr/lib/lib* $(FFLIB)
	cp -a $(BUILDROOT)/output/target/usr/lib/libtirpc* $(FFLIB)
	cp -a $(BUILDROOT)/output/target/lib/libutil* $(FFLIB)
	cp -a $(BUILDROOT)/output/target/lib/libatomic* $(FFLIB)
	cp $(BUILDROOT)/output/target/bin/busybox $(FFBIN)/busybox-i686
	if [ -d $(STAGEDIR)/usr/local/bin ]; then mv $(STAGEDIR)/usr/local/bin/* $(FFBIN); fi
	if [ -d $(STAGEDIR)/usr/local/sbin ]; then mv $(STAGEDIR)/usr/local/sbin/* $(FFBIN); fi
	if [ -d $(STAGEDIR)/usr/sbin ]; then mv $(STAGEDIR)/usr/sbin/* $(FFBIN); fi
	rm -rf $(STAGEDIR)/usr/local $(STAGEDIR)/usr/share $(STAGEDIR)/usr/include $(STAGEDIR)/usr/lib/pkgconfig  $(STAGEDIR)/usr/lib/*.a $(STAGEDIR)/usr/sbin
	@cd $(FFBIN); \
	    for f in `ls | xargs file | grep ELF | sed -e 's/:.*//'`; do\
		strip $$f;\
		mv $$f $(FFEXEC)/$$f;\
		ln -sv ../lib/ff/exec/ffwrap $$f;\
	    done

buildroot:	_TGT=base

$(BASE_PACKAGES):
	make -C $@ $(_TGT)

clean:
	make -C ffritz clean

cleanall:
	make -C ffritz cleanall
