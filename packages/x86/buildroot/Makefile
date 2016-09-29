
PKGTOP	= $(shell cd ../..; pwd)
DLDIR	= $(PKGTOP)/dl

export PATH := $(TOOLCHAIN):$(PATH)

URL	= $(shell cat $(PKGTOP)/url-buildroot)
FILE	= $(DLDIR)/$(shell basename $(URL))

all:	build/Makefile
	@make -C build toolchain
	@make -C build boost
	@make -C build zlib
	@make -C build expat

$(FILE):
	@cd $(DLDIR); wget $(URL)

build/Makefile:    $(FILE)
	@mkdir -p build
	@cd build; tar xf $(FILE) --strip-components=1
	@cp mod/config build/.config

clean:
	rm -rf build

distclean:
	rm -rf build
	rm -f $(FILE)
