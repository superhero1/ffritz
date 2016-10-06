
all:	sndlist usbplay

sndlist:    sndlist.c
	i686-buildroot-linux-uclibc-gcc -o sndlist sndlist.c -std=c99 -I ../../libmaru -L ../../libmaru -lmaru

usbplay:    usbplay.c
	i686-buildroot-linux-uclibc-gcc -o usbplay usbplay.c -std=c99 -I ../../libmaru -L ../../libmaru -lmaru


