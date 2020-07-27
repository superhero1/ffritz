#
# Options:
#  INSTDIR   [/usr/local]
#  BUILDDIR  [.]
#

CFLAGS += -Wall -Wno-format-security -O2

INSTDIR  ?= /usr/local
BUILDDIR ?= .

all: $(BUILDDIR)/uimg

uimg.c:	uimg.h

$(BUILDDIR)/uimg:	uimg.c
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(BUILDDIR)/uimg

install: all
	@mkdir -p $(INSTDIR)/bin
	cp $(BUILDDIR)/uimg $(INSTDIR)/bin
