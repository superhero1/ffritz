# Default make for arm
#
# Build ffritz package, which builds other packages on demand
#
MODDIR		= $(shell readlink -f ../../arm/mod)
STAGEDIR	= $(shell readlink -f .)/staging

BASE_PACKAGES=buildroot dropbear athtool ffdaemon

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
	mkdir -p $(FFLIB)
	mkdir -p $(FFEXEC)
	mkdir -p $(FFBIN)
	mkdir -p $(STAGEDIR)/var/empty
	rm -f  $(STAGEDIR)/usr/lib/*.a
	rm -rf  $(STAGEDIR)/etc/ssl/man
	make -C dropbear install DESTDIR=$(STAGEDIR) INSTDIR=$(STAGEDIR)
	make -C athtool install DESTDIR=$(STAGEDIR)/usr INSTDIR=$(STAGEDIR)/usr
	make -C ffdaemon install INSTDIR=$(STAGEDIR)/usr
	cd buildroot/build/output/target/usr/bin; cp `ls -f tcp* gdb* strace ltrace` $(FFBIN)
	cd buildroot/build/output/target/usr/sbin; cp `ls -f tcp*` $(FFBIN)
	cd buildroot/build/output/target/lib; cp -af libuClibc-* $(FFLIB)
	cd buildroot/build/output/target/usr/lib; cp -af `ls -f libpcap* libelf* libstdc++* libdw* libncurses*` $(FFLIB)
	if [ -d $(STAGEDIR)/usr/local/bin ]; then mv $(STAGEDIR)/usr/local/bin/* $(FFBIN); fi
	if [ -d $(STAGEDIR)/usr/local/sbin ]; then mv $(STAGEDIR)/usr/local/sbin/* $(FFBIN); fi
	if [ -d $(STAGEDIR)/usr/sbin ]; then mv $(STAGEDIR)/usr/sbin/* $(FFBIN); fi
	rm -rf $(STAGEDIR)/usr/local $(STAGEDIR)/usr/share $(STAGEDIR)/usr/include $(STAGEDIR)/usr/lib/pkgconfig  $(STAGEDIR)/usr/lib/*.a $(STAGEDIR)/usr/sbin
	rm -rf $(STAGEDIR)/usr/bin/ssh
	@cd $(FFBIN); \
	    for f in `ls`; do \
		mv $$f $(FFEXEC)/$$f;\
		ln -sv ../lib/ff/exec/ffwrap $$f;\
	    done
	cd $(FFLIB); ln -s libuClibc-* libc.so.0
	cd $(FFLIB); ln -s libuClibc-* libc.so.1

buildroot:	_TGT=base

$(BASE_PACKAGES):
	make -C $@ $(_TGT)

clean:
	make -C ffritz clean

cleanall:
	make -C ffritz cleanall
