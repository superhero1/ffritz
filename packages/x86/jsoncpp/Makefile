ARCHDIR=$(shell while test -f arch.mk && echo $$PWD && exit 0; test $$PWD != /; do cd ..; done)
include $(ARCHDIR)/../paths.mk

DESTDIR     = $(TOPDIR)/output

URL     = $(shell cat $(PKGTOP)/url-jsoncpp)
FILE    = $(DLDIR)/$(shell basename $(URL))

OBJECTS = json_reader.o json_writer.o json_value.o
VPATH = $(BUILDDIR)/src/lib_json

export PATH := $(PATH):$(TOOLCHAIN)

all:	$(BUILDDIR)
	@make libjsoncpp.a
	@install -vd $(SYSROOT)/usr/include/json
	@install -vC $(BUILDDIR)/include/json/* $(SYSROOT)/usr/include/json
	@install -vC jsoncpp.pc $(SYSROOT)/usr/lib/pkgconfig
	@install -vC libjsoncpp.a $(SYSROOT)/usr/lib

libjsoncpp.a:	$(OBJECTS)
	ar crus $@ $^

.cpp.o:
	i686-buildroot-linux-gnu-c++ -I $(BUILDDIR)/include -c $<

$(FILE):
	@cd $(DLDIR); wget $(URL)

build:	$(FILE)
	@mkdir -p build
	@cd build; tar xf $(FILE) --strip-components=1
	@touch $(BUILDDIR)/configure

clean:
	rm -rf build $(OBJECTS) libjsoncpp.a
