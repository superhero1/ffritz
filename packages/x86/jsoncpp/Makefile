PKGTOP	= $(shell cd ../..; pwd)
DLDIR	= $(PKGTOP)/dl

TOPDIR	    = $(shell pwd)
BUILDROOT   = $(shell cd ../buildroot/build; pwd)
TOOLCHAIN   = $(shell cd $(BUILDROOT)/output/host/usr/bin/; pwd)
DESTDIR	    = $(TOPDIR)/output
SYSROOT	    = $(BUILDROOT)/output/host/usr/i686-buildroot-linux-uclibc/sysroot

URL	= $(shell cat $(PKGTOP)/url-jsoncpp)
FILE	= $(DLDIR)/$(shell basename $(URL))

OBJECTS = json_reader.o json_writer.o json_value.o
VPATH =	build/src/lib_json

export PATH := $(PATH):$(TOOLCHAIN)

all:	libjsoncpp.a
	cp -arv build/include/json $(SYSROOT)/usr/include
	cp -av jsoncpp.pc $(SYSROOT)/usr/lib/pkgconfig
	cp -av libjsoncpp.a $(SYSROOT)/usr/lib

libjsoncpp.a: $(OBJECTS)
	ar crus $@ $^

.cpp.o:
	i686-buildroot-linux-uclibc-c++ -I build/include -c $<

$(OBJECTS:%.o=$(VPATH)/%.cpp): build

$(FILE):
	@cd $(DLDIR); wget $(URL)

build:    $(FILE)
	@mkdir -p build
	@cd build; tar xf $(FILE) --strip-components=1
	@touch build/configure

clean:
	rm -rf build $(OBJECTS) libjsoncpp.a
