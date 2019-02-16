
FFHOME		= /var/media/ftp/ffritz
CONFTYPE	= autoconf

PKG_CONFIGURE_FLAGS = --datarootdir=$(FFHOME)/share --sysconfdir=$(FFHOME)/etc

include ../../package.mk

BIN=$(wildcard $(DESTDIR)/usr/bin/*)
PLUG=$(shell find $(DESTDIR)/$(FFHOME)/share/upmpdcli -maxdepth 1 -type f 2>/dev/null )

install: 
	@mkdir -p $(DESTDIR)/bin $(DESTDIR)/share/upmpdcli/cdplugins $(DESTDIR)/etc
	@$(foreach f,$(BIN),$(call _INST,$(f),$(DESTDIR)/bin))
	@$(foreach f,$(PLUG),$(call _INST,$(f),$(DESTDIR)/share/upmpdcli))
	@rm -rf $(DESTDIR)/var

