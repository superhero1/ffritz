## pihole build
##
############################################################################

ARCHDIR  := $(shell while test -f arch.mk && echo $$PWD && exit 0; test $$PWD != /; do cd ..; done)
include $(ARCHDIR)/arch.mk

WD       := $(shell pwd)
BUILDDIR := $(WD)/build
TPL      := $(BUILDDIR)/pihole-template.tar.gz
SRCDIR   := $(WD)/src

ifeq ($(DESTDIR),)
DESTDIR=output
endif


all: $(TPL)

$(TPL): $(call NEWEST,src)
	mkdir -p `dirname $@`
	cd $(SRCDIR); tar cfz $@ .

install:
	mkdir -p $(DESTDIR)/bin 
	mkdir -p $(DESTDIR)/share/pihole
	cp *.sh timeout pihole-localhosts $(DESTDIR)/bin
	cp pihole $(TPL) $(DESTDIR)/share/pihole

clean:
	rm -f $(TPL)
