
CONFTYPE	= autoconf
MAKE_OPTIONS	= PROGRAMS="dropbear dropbearkey dropbearconvert dbclient scp" SCPPROGRESS=1 CPPFLAGS=-DUSE_DEV_PTMX
PKG_CONFIGURE_FLAGS = --with-gnu-ld --disable-nls --disable-pam --disable-syslog --enable-shadow --disable-lastlog --disable-utmp --disable-utmpx --disable-wtmp --disable-wtmpx --disable-loginfunc --disable-pututline --disable-pututxline --enable-bundled-libtom --disable-largefile --disable-zlib --disable-openpty --disable-harden 
MAKE_INSTALL_OPTIONS = PROGRAMS="dropbear dropbearkey dropbearconvert dbclient scp"

include ../../package.mk
