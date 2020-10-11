# Rebuild binaries integrated into the rootfs modifications
#

# force buildroot version, required to support all later OS
#
BR_VERSION  = -2019.05

ARCHDIR=$(shell while test -f arch.mk && echo $$PWD && exit 0; test $$PWD != /; do cd ..; done)
include $(ARCHDIR)/arch.mk

STAGEDIR	:= $(TOPDIR)/output

TOOLS           := dropbear privatekeypassword libmultid
#TOOLS           += tools
BASE_PACKAGES   := $(BRBUILDDIR) $(TOOLS) 
TOOLS_INSTALL   := $(TOOLS:%=%-install)

FFLIB	= $(STAGEDIR)/usr/lib/ff/lib
FFEXEC	= $(STAGEDIR)/usr/lib/ff/exec
FFBIN	= $(STAGEDIR)/usr/bin

.PHONY:	$(BRBUILDDIR) $(TOOLS) $(STAGEDIR) $(TOOLS_INSTALL)

all:	$(BASE_PACKAGES) base-install

base-install: stage
	cp -arf $(STAGEDIR)/* $(PLAT_OVERLAY)

$(STAGEDIR):
	@$(call RMDIR,$(STAGEDIR))
	@mkdir -p $(STAGEDIR)/lib
	@mkdir -p $(FFLIB)
	@mkdir -p $(FFEXEC)
	@mkdir -p $(FFBIN)
	@mkdir -p $(STAGEDIR)/var/empty

openssl:
	@make -e -C $(BUILDROOT)/output/build/libopenssl* install INSTALL_PREFIX=$(STAGEDIR) DESTDIR=$(STAGEDIR)
	@rm -f  $(STAGEDIR)/usr/bin/c_rehash $(STAGEDIR)/etc/ssl/openssl.cnf
	@rm -f  $(STAGEDIR)/usr/lib/*.a
	@rm -f  $(STAGEDIR)/usr/lib/*.la
	@rm -rf  $(STAGEDIR)/etc/ssl/man

socat:  $(BUILDROOT)/output/target/usr/bin/socat
	@cp -a $(BUILDROOT)/output/target/usr/bin/socat $(FFBIN)
	

INSTALL_DEST=$(STAGEDIR)/usr
dropbear-install: INSTALL_DEST=$(STAGEDIR)

$(TOOLS_INSTALL):
	@make -C $(ARCHDIR)/$(@:%-install=%) install DESTDIR=$(INSTALL_DEST) prefix=/usr BR_VERSION=$(BR_VERSION)

bin:
	@echo +++ collecting binaries to $(FFBIN)
	@test -d $(STAGEDIR)/usr/local/bin && mv $(STAGEDIR)/usr/local/bin/* $(FFBIN); true
	@test -d $(STAGEDIR)/usr/local/sbin && mv $(STAGEDIR)/usr/local/sbin/* $(FFBIN); true
	@test -d $(STAGEDIR)/usr/sbin && mv $(STAGEDIR)/usr/sbin/* $(FFBIN); true
	@rm -f $(FFBIN)/ffdaemon
#	@mv $(FFBIN)/ffwrap $(FFEXEC) || true

lib:
	@echo +++ collecting libraries to $(FFLIB)
	@mv $(STAGEDIR)/usr/lib/lib* $(FFLIB)
	@cp -a $(BUILDROOT)/output/target/usr/lib/libtirpc* $(FFLIB)
	@cp -a $(BUILDROOT)/output/target/lib/libutil* $(FFLIB)
	@cp -a $(BUILDROOT)/output/target/lib/libatomic* $(FFLIB)
	@rm -f $(FFLIB)/*.a $(FFLIB)/*.la

cleanup:
	@echo +++ clean staging
	@$(call RMDIR,$(STAGEDIR)/usr/local)
	@$(call RMDIR,$(STAGEDIR)/usr/share)
	@$(call RMDIR,$(STAGEDIR)/usr/include)
	@$(call RMDIR,$(STAGEDIR)/usr/lib/pkgconfig)
	@$(call RMDIR,$(STAGEDIR)/usr/sbin)
	@$(call RMDIR,$(STAGEDIR)/lib)

move:
	@echo +++ move ELF binaries from $(FFBIN) to $(FFEXEC)
	@mv $(call LS_ELF_ALL,$(FFBIN)) $(FFEXEC)

link:
	@echo +++ relink binaries from $(FFEXEC) to $(FFBIN)
	@$(call LINK_TO_ONE,$(call LS_ELF,$(FFEXEC)),$(FFBIN),../lib/ff/exec/ffwrap)
	@echo ../lib > $(FFEXEC)/.fflibs
#	@ls -lL $(FFBIN) >/dev/null

stage:	$(STAGEDIR) openssl $(TOOLS_INSTALL) socat bin lib cleanup move link

$(BRBUILDDIR):	
	make -C $@ base

$(TOOLS):
	make -C $(ARCHDIR)/$@ BR_VERSION=$(BR_VERSION)

clean:
	$(call RMDIR,$(STAGEDIR))
