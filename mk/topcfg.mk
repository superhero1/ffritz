# requires: REPODIR, DLDIR
#
# Provides:
# SOC		puma6 or puma7
# FWVER		firmware version aa.ii
# FWNUM 	firmware version aaii
# FWMAJ 	major version
# FWMIN 	minor version
# BETA  	beta version number
# MODEL 	6490, 6590, 6591, 6660
# BR_VERSION	BR version suffix
#
#
# 
include $(REPODIR)/conf.mk.dfl
ifneq ($(NO_USER_CONF),1)
-include $(REPODIR)/conf.mk
endif

# Determine image type
#
ITYPE=$(shell echo $(URL) | sed -e 's/.*\.//')
INAME=$(shell basename $(URL))
DLIMAGE=$(DLDIR)/$(INAME)

ifeq ($(ITYPE),zip)
# Fetch and extract labor zip
INAME=$(shell mkdir -p $(DLDIR); if [ ! -f $(DLIMAGE) ]; then wget -O $(DLIMAGE) $(URL); fi; unzip -l $(DLIMAGE) | grep image | sed -e 's/.*\(FRITZ.*.image\)/\1/')
ifeq ($(INAME),)
error $(URL) is not a valid firmware zip
endif
endif

FWVER=$(shell echo $(INAME) | sed -e 's/.*\([0-9]*.\.[0-9]*\).*\.image/\1/')
BETA=$(shell echo $(INAME) | sed -e 's/.*-\([0-9]*\)-Lab.*/-\1/' | grep -v image)
MODEL=$(shell echo $(INAME) | sed -e 's/.*_\(....\)_Cable.*/\1/')
FWNUM=$(subst .,,$(FWVER))

ifeq ($(FWVER),)
$(error Could not determine firmware version ($(INAME) missing?))
endif

FWMAJ=$(shell echo $(FWVER) | sed -e 's/\..*//')
FWMIN=$(shell echo $(FWVER) | sed -e 's/.*\.//')

ifeq ($(MODEL),6591)
SOC=puma7
endif
ifeq ($(MODEL),6660)
SOC=puma7
endif

ifeq ($(MODEL),6490)
SOC=puma6
endif
ifeq ($(MODEL),6590)
SOC=puma6
endif

ifeq ($(shell test $(FWMAJ) -lt 7),0)
$(error sorry, firmware too old, try the fritzos6 branch)
endif

ifeq ($(SOC),puma7)
ifeq ($(BR_VERSION),)
ifeq ($(shell test $(FWMAJ) -eq 7 -a $(FWMIN) -lt 19 ; echo $$?),0)
BR_VERSION=-2019.05
else # >= 7.19
BR_VERSION=-2020.02
endif
endif

ifeq ($(BR_VERSION),-2020.02)
ifeq ($(shell test $(FWMAJ) -eq 7 -a $(FWMIN) -lt 19 ; echo $$?),0)
$(error This toolchain will likely not work with firmware < 7.19)
endif
ifeq ($(PARALLEL),)
PARALLEL=$(shell cat /proc/cpuinfo | grep processor | wc -l)
endif # PARALLEL
endif # 2020.02

else #puma6

BR_VERSION=-2019.05-p6
endif

ARM_IMG_VERSION=$(shell cat $(REPODIR)/packages/arm/ffritz/version)
ATOM_IMG_VERSION=$(shell cat $(REPODIR)/packages/x86/ffritz/version)

ARM_EXT_IMAGE = ffritz-arm-$(ARM_IMG_VERSION)-$(SOC)-fos7.tar.gz
APP_IMAGE     = ffritz-app-$(ATOM_IMG_VERSION)-$(SOC)-fos7$(FOS_SUB_VERSION).tar
