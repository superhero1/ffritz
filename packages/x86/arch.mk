ARCH		= x86
HOST		= i686-buildroot-linux-gnu
CROSS		= i686-buildroot-linux-gnu-
CROSS_COMPILE	= i686-buildroot-linux-gnu-
SQ_ROOT		= $(realpath $(ARCHDIR)/../../atom/squashfs-root)

FRITZOS_LIB	= $(realpath $(SQ_ROOT)/lib/)

include $(ARCHDIR)/../../mk/ffconfig.mk
