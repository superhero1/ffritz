# optional: 
#
#
TARGETS = ffdaemon ffwrap

all:	$(TARGETS)

ffdaemon:	ffdaemon.o daemon.o
	$(CC) -o $@ $^ -std=c99 

ffwrap.o: ffwrap-help.c

ffwrap-help.c: ffwrap.c
	@echo generating $@
	@echo 'static char *help =' > $@
	@cat $< |\
	sed -n -e '/DOC_START/,/DOC_END/{ /DOC_START/d; /DOC_END/d; p; }' |\
	sed -e 's/^[ ]*\* //' -e 's/\([%"]\)/\1\1/g' |\
	sed -e 's/\(.*\)/"\1\\n"/' >> $@
	@echo ';' >> $@

ffwrap-help.c: Makefile

install: $(TARGETS)
	mkdir -p $(DESTDIR)/bin
	install $^ $(DESTDIR)/bin
	
clean:
	rm -f $(TARGETS) *.o

