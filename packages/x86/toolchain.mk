ifeq ($(BUILDROOT),)
BUILDROOT   = $(shell cd ../buildroot/build; pwd)
endif

ifeq ($(BUILDROOT),)
$(error BUILDROOT (buildroot top directory) not defined)
endif

ifeq ($(CROSS_COMPILE),)
CROSS_COMPILE=i686-buildroot-linux-gnu-
endif


