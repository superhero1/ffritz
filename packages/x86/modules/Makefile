
KERNELS	= 3.12.59 3.12.74
MODULES = wireguard

ifeq ($(DESTDIR),)
DESTDIR = $(shell pwd)/output
endif

TMPDIR = $(shell pwd)/modules/$(KERNEL)

KDIRS	= $(KERNELS:%=../kernel_%)

.PHONY:	$(KDIRS)

all:	$(KDIRS)
	for k in $(KERNELS); do \
	 for m in $(MODULES); do \
	  make KERNEL=$$k MODULE=$$m module; \
         done; \
	done

$(KDIRS):
	make -C $@

module:
	test -d ../kernel_$(KERNEL)/build
	mkdir -p $(TMPDIR)
	make -C ../$(MODULE) KERNELDIR=$(shell cd ../kernel_$(KERNEL)/build; pwd) DESTDIR=$(TMPDIR) install

install: all
	mkdir -p $(DESTDIR)/bin
	for k in $(KERNELS); do \
	 mkdir -p $(DESTDIR)/lib/modules/$$k; \
	 cp -arv $(TMPDIR)/$$k/lib/* $(DESTDIR)/lib/modules/$$k; \
	 cp -av $(TMPDIR)/$$k/bin/* $(DESTDIR)/bin; \
	done

clean:
	for k in $(KERNELS); do make -C ../kernel_$$k clean; done
	rm -rf modules output
