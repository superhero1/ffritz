

$(RELDIR)/$(FWFILE): $(RELDIR) armfs atomfs 
	@rm -rf $(PLAT_TMP)/var
	@cd $(PLAT_TMP); tar xf $(ORIG)
	@echo "PACK   $(RELDIR)/$(FWFILE)"
	@cp -f $(ARM_TMP)/filesystem.image $(ARM_ROOTIMG)
	@cp $(ARM_BASE)/mod/usr/bin/switch_bootbank $(PLAT_TMP)/var
	@cp -f $(ATOM_TMP)/filesystem.image $(ATOM_ROOTIMG)
#	cd $(PLAT_TMP); patch -p0 < $(PLAT_BASE)/install.p
	@cd $(PLAT_TMP); $(TAR) cf $(RELDIR)/$(FWFILE) var
	@echo
	@echo +++ Done +++
