ARCH		= arm
HOST		= armeb-buildroot-linux-gnueabi
CROSS		= armeb-buildroot-linux-gnueabi-
CROSS_COMPILE	= armeb-buildroot-linux-gnueabi-
SQ_ROOT		= $(realpath $(ARCHDIR)/../../arm/squashfs-root)

FRITZOS_LIB	= $(realpath $(PKGTOP)/../arm/squashfs-root/lib)

include $(ARCHDIR)/../../mk/ffconfig.mk
