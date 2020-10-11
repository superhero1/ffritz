ARCHDIR=$(shell while test -f arch.mk && echo $$PWD && exit 0; test $$PWD != /; do cd ..; done)
include $(ARCHDIR)/arch.mk


ifeq ($(BR_VERSION),-2019.05)
KERNELS	= 3.12.59 3.12.74
else
KERNELS	= 4.9.199
MODULES = wireguard
endif

ifeq ($(DESTDIR),)
DESTDIR = $(shell pwd)/output
endif

TMPDIR := $(shell pwd)/modules/$(KERNEL)
KDIRS  := $(KERNELS:%=../kernel_%)

BUILDDIR = build$(BR_VERSION)

.PHONY:	$(KDIRS)

all:	$(KDIRS)
	for k in $(KERNELS); do \
	 for m in $(MODULES); do \
	  make -j KERNEL=$$k MODULE=$$m module; \
         done; \
	done

$(KDIRS):
	make -C $@

module:
	test -d ../kernel_$(KERNEL)/$(BUILDDIR)
	mkdir -p $(TMPDIR)
	make -j -C ../$(MODULE) KERNELDIR=$(realpath ../kernel_$(KERNEL)/$(BUILDDIR)) DESTDIR=$(TMPDIR) BUILDDIR=build_$(KERNEL)$(BR_VERSION) all
	make -j -C ../$(MODULE) KERNELDIR=$(realpath ../kernel_$(KERNEL)/$(BUILDDIR)) DESTDIR=$(TMPDIR) BUILDDIR=build_$(KERNEL)$(BR_VERSION) install

cleanmodule:
	make -C ../$(MODULE) KERNELDIR=$(realpath ../kernel_$(KERNEL)/$(BUILDDIR)) DESTDIR=$(TMPDIR) BUILDDIR=build_$(KERNEL)$(BR_VERSION) clean

install: all
	mkdir -p $(DESTDIR)/bin
	for k in $(KERNELS); do \
	 make -C ../kernel_$$k DESTDIR=$(DESTDIR)/lib/modules/$$k install-modules && \
	 mkdir -p $(DESTDIR)/lib/modules/$$k && \
	 cp -arv $(TMPDIR)/$$k/lib/* $(DESTDIR)/lib/modules/$$k; \
	done

clean:
	for k in $(KERNELS); do \
	 for m in $(MODULES); do \
	  make KERNEL=$$k MODULE=$$m cleanmodule; \
         done; \
	 make -C ../kernel_$$k clean; \
	done
