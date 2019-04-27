# Default make for atom
#
# Build ffritz package, which builds other packages on demand
#
MODDIR		= $(shell readlink -f ../../atom/mod)
STAGEDIR	= $(shell readlink -f .)/staging

BASE_PACKAGES=buildroot dropbear privatekeypassword #unionfs-fuse

.PHONY:	$(BASE_PACKAGES)

all:
	make -C ffritz

base:	$(BASE_PACKAGES)

base-install:
	rm -rf $(STAGEDIR)
	mkdir -p $(STAGEDIR)
	make -e -C buildroot/build/output/build/libopenssl* install INSTALL_PREFIX=$(STAGEDIR) DESTDIR=$(STAGEDIR)
	mkdir -p $(STAGEDIR)/usr/lib/openssl/exec
	mkdir -p $(STAGEDIR)/usr/lib/openssl/lib
	mv $(STAGEDIR)/usr/lib/*so*  $(STAGEDIR)/usr/lib/openssl/lib
	rm -f  $(STAGEDIR)/usr/bin/c_rehash $(STAGEDIR)/etc/ssl/openssl.cnf
	make -C privatekeypassword install DESTDIR=$(STAGEDIR)/usr
	make -C dropbear install DESTDIR=$(STAGEDIR) prefix=/usr
#	make -C unionfs-fuse install DESTDIR=$(STAGEDIR)
#	make -e -C buildroot/build/output/build/libfuse-*/ install DESTDIR=$(STAGEDIR)
	mkdir -p $(STAGEDIR)/var/empty
	cp buildroot/build/output/build/busybox-*/busybox $(STAGEDIR)/usr/bin/busybox-i686
	cp -a buildroot/build/output/target/usr/lib/libtirpc.so* $(STAGEDIR)/usr/lib
	if [ -d $(STAGEDIR)/usr/local/bin ]; then mv $(STAGEDIR)/usr/local/bin/* $(STAGEDIR)/usr/bin; fi
	if [ -d $(STAGEDIR)/usr/local/sbin ]; then mv $(STAGEDIR)/usr/local/sbin/* $(STAGEDIR)/usr/bin; fi
	if [ -d $(STAGEDIR)/usr/sbin ]; then mv $(STAGEDIR)/usr/sbin/* $(STAGEDIR)/usr/bin; fi
	rm -rf $(STAGEDIR)/usr/local $(STAGEDIR)/usr/share $(STAGEDIR)/usr/include $(STAGEDIR)/usr/lib/pkgconfig  $(STAGEDIR)/usr/lib/*.a $(STAGEDIR)/usr/sbin
	strip $(STAGEDIR)/usr/bin/*
	mv $(STAGEDIR)/usr/bin/openssl  $(STAGEDIR)/usr/lib/openssl/exec
	mkdir -p $(STAGEDIR)/lib
	cp -ar buildroot/build/output/target/lib/libuClibc-*.so $(STAGEDIR)/lib/
	cd $(STAGEDIR)/lib; ln -s libuClibc-*.so libc.so.0
	cp -arf $(STAGEDIR)/* $(MODDIR)

buildroot:	_TGT=base

$(BASE_PACKAGES):
	make -C $@ $(_TGT)

clean:
	make -C ffritz clean

cleanall:
	make -C ffritz cleanall
