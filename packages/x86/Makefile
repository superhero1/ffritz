# Default make for atom
#
# Build ffritz package, which builds other packages on demand
#
MODDIR		= $(shell cd ../../atom/mod; pwd)
STAGEDIR	= $(shell pwd)/staging

BASE_PACKAGES=buildroot dropbear privatekeypassword openssh

.PHONY:	$(BASE_PACKAGES)

all:
	make -C ffritz

base:	$(BASE_PACKAGES)

base-install:
	rm -rf $(STAGEDIR)
	make -e -C buildroot/build/output/build/openssl-1.0.2h install INSTALL_PREFIX=$(STAGEDIR)
	mkdir -p $(STAGEDIR)/usr/lib/openssl/exec
	mkdir -p $(STAGEDIR)/usr/lib/openssl/lib
	mv $(STAGEDIR)/usr/lib/*so*  $(STAGEDIR)/usr/lib/openssl/lib
	rm -f  $(STAGEDIR)/usr/bin/c_rehash $(STAGEDIR)/etc/ssl/openssl.cnf
	make -C privatekeypassword install DESTDIR=$(STAGEDIR)/usr
	make -C dropbear install DESTDIR=$(STAGEDIR) prefix=/usr
#	make -C openssh install DESTDIR=$(STAGEDIR)
#	mv $(STAGEDIR)/etc/openssh $(STAGEDIR)/etc/openssh.dfl
#	mkdir -p $(STAGEDIR)/var/empty
	cp buildroot/build/output/build/busybox-1.24.2/busybox $(STAGEDIR)/usr/bin/busybox-i686
	if [ -d $(STAGEDIR)/usr/local/bin ]; then mv $(STAGEDIR)/usr/local/bin/* $(STAGEDIR)/usr/bin; fi
	if [ -d $(STAGEDIR)/usr/local/sbin ]; then mv $(STAGEDIR)/usr/local/sbin/* $(STAGEDIR)/usr/bin; fi
	if [ -d $(STAGEDIR)/usr/sbin ]; then mv $(STAGEDIR)/usr/sbin/* $(STAGEDIR)/usr/bin; fi
	rm -rf $(STAGEDIR)/usr/local $(STAGEDIR)/usr/share $(STAGEDIR)/usr/include $(STAGEDIR)/usr/lib/pkgconfig  $(STAGEDIR)/usr/lib/*.a $(STAGEDIR)/usr/sbin
	strip $(STAGEDIR)/usr/bin/*
	mv $(STAGEDIR)/usr/bin/openssl  $(STAGEDIR)/usr/lib/openssl/exec
	cp -arf $(STAGEDIR)/* $(MODDIR)

buildroot:	_TGT=base

$(BASE_PACKAGES):
	make -C $@ $(_TGT)

clean:
	make -C ffritz clean

cleanall:
	make -C ffritz cleanall
