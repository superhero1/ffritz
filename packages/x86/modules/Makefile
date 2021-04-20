ARCHDIR=$(shell while test -f arch.mk && echo $$PWD && exit 0; test $$PWD != /; do cd ..; done)
include $(ARCHDIR)/arch.mk

MODULES_ALL=
ifeq ($(BR_VERSION),-2019.05)
KERNELS	= 3.12.59 3.12.74
else
KERNELS	= 4.9.199 4.9.250
endif
MODULES_4.9.199=wireguard
MODULES_4.9.250=wireguard

ifeq ($(DESTDIR),)
DESTDIR = $(shell pwd)/output
endif

TMPDIR := $(shell pwd)/modules/$(KERNEL)
KDIRS  := $(KERNELS:%=../kernel_%)

BUILDDIR = build$(BR_VERSION)

.PHONY:	$(KDIRS)

all:	$(KDIRS)
	for k in $(KERNELS); do \
	  make -j KERNEL=$$k module; \
	done

$(KDIRS):
	make -C $@

module:
	test -d ../kernel_$(KERNEL)/$(BUILDDIR)
	for m in $(MODULES_ALL) $(MODULES_$(KERNEL)); do \
		make -j -C ../$$m KERNELDIR=$(realpath ../kernel_$(KERNEL)/$(BUILDDIR)) DESTDIR=$(TMPDIR) BUILDDIR=build_$(KERNEL)$(BR_VERSION) all;\
	done
	for m in $(MODULES_ALL) $(MODULES_$(KERNEL)); do \
		make -j -C ../$$m KERNELDIR=$(realpath ../kernel_$(KERNEL)/$(BUILDDIR)) DESTDIR=$(TMPDIR) BUILDDIR=build_$(KERNEL)$(BR_VERSION) install;\
	done

cleanmodule:
	make -C ../$(MODULE) KERNELDIR=$(realpath ../kernel_$(KERNEL)/$(BUILDDIR)) DESTDIR=$(TMPDIR) BUILDDIR=build_$(KERNEL)$(BR_VERSION) clean

install: all
	mkdir -p $(DESTDIR)/bin
	for k in $(KERNELS); do \
	 mkdir -p $(DESTDIR)/lib/modules/$$k/lib && \
	 make -C ../kernel_$$k DESTDIR=$(DESTDIR)/lib/modules/$$k install-modules && \
	 test -d $(TMPDIR)/$$k/lib && cp -arv $(TMPDIR)/$$k/lib/* $(DESTDIR)/lib/modules/$$k || true; \
	done

clean:
	for k in $(KERNELS); do \
	 for m in $(MODULES); do \
	  make KERNEL=$$k MODULE=$$m cleanmodule; \
         done; \
	 make -C ../kernel_$$k clean; \
	done
