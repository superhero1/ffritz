ARCH		= x86
HOST		= i686-buildroot-linux-uclibc
CROSS		= i686-buildroot-linux-uclibc-

FRITZOS_LIB	= $(shell cd $(PKGTOP)/../atom/squashfs-root/lib/; pwd)
