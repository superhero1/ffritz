## Additional modules in package/x86 to build and install
#
MODULES   += nfs
MODULES   += bt
MODULES   += prtg
MODULES   += modules
MODULES   += wireguard-tools
MODULES   += unfs
MODULES   += cash
MODULES   += ffad

# Options for pihole. Requires BR_VERSION other than 2019-05 in toplevel conf.mk
ifneq ($(BR_VERSION),-2019.05)
MODULES   += ftl
MODULES   += libmultid
MODULES   += pi-hole
endif
