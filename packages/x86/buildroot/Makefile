BUILDROOT_TARGETS   = boost zlib expat libmad libusb libfuse mpd strace tcpdump tcpreplay openssl

include ../../buildroot.mk

arch-patches:
	diff -qN mod/config build/.config || cp mod/config build/.config
	diff -qN mod/alsa-lib.mk build/package/alsa-lib/alsa-lib.mk || cp mod/alsa-lib.mk build/package/alsa-lib/alsa-lib.mk
	diff -qN mod/uClibc-0.9.33.config build/toolchain/uClibc/uClibc-0.9.33.config || cp mod/uClibc-0.9.33.config build/toolchain/uClibc/uClibc-0.9.33.config
	diff -qN mod/toolchain_gcc_Config.in build/toolchain/gcc/Config.in || cp mod/toolchain_gcc_Config.in build/toolchain/gcc/Config.in
	
