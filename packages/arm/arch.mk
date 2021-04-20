ARCH		= arm
HOST		= armeb-buildroot-linux-gnueabi
CROSS		= armeb-linux-
CROSS_COMPILE	= armeb-linux-
SQ_ROOT		= $(realpath $(ARCHDIR)/../../arm/squashfs-root)

FRITZOS_LIB	= $(realpath $(PKGTOP)/../arm/squashfs-root/lib)

include $(ARCHDIR)/../../mk/ffconfig.mk
