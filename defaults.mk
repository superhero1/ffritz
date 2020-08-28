include conf.mk.dfl
-include conf.mk

ifeq ($(findstring 6490,$(URL)),6490)
SOC=puma6
endif
ifeq ($(findstring 6590,$(URL)),6490)
SOC=puma6
endif

ifeq ($(findstring 6591,$(URL)),6591)
SOC=puma7
endif
ifeq ($(findstring 6660,$(URL)),6591)
SOC=puma7
endif

ifeq ($(SOC),puma6)
ifeq ($(BR_VERSION),)
BR_VERSION=-2019.05-p6
endif
endif

ifeq ($(SOC),puma7)
ifeq ($(BR_VERSION),)
BR_VERSION=-2019.05
endif
endif

ifeq ($(SOC),)
$(error unknown model)
endif


