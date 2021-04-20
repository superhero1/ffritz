ARCH		= x86
HOST		= i686-buildroot-linux-gnu
CROSS		= i686-linux-
CROSS_COMPILE	= i686-linux-
SQ_ROOT		= $(realpath $(ARCHDIR)/../../atom/squashfs-root)

FRITZOS_LIB	= $(realpath $(SQ_ROOT)/lib/)

include $(ARCHDIR)/../../mk/ffconfig.mk
