This is a repository containing my modifications for FritzBox-6490 Cable.
You still need the original image (original_141.06.50.tar) to build a
modified install image, and you need a way to upgrade.



NOTES
=====

Toolchain
---------
Cross-compile toolchain for ARM is buildroot-2013.08 (it's the first one i looked for that
matches the uClibc version used in FB frimware 6.50).

There seems to be a problem with locale support, since dropber binaries complain
about missing symbols (caused by usage of ctype.h macros like isalnum, toupper, ...).

Tools
-----
dropbear (dropbear-2016.74/)
    - Need to apply some patches to emlinate isspace, isalnum, toupper (see above).

    - To start (debug, in chroot):
    /usr/local/sbin/dropbear -R -F -E -B

    ssh root@192.168.0.1 /sbin/ar7login


DEVELOPMENT
===========
It's convenient to mirror the root file system to /var/media/ftp/root and chroot into it:

cd /var/media/ftp/root
mount --bind / /var/media/ftp/root
tar cf root.tar root
umount root

tar xf root.tar	    (best on ATOM for speed)
mkdir root/var/tmp
cp -a /var/tmp/* root/var/tmp

mount --bind /sys root/sys
mount --bind /dev root/dev
mount --bind /proc root/proc

chroot root


Usage
=====

ssh
---
To start the dropbear daemon you need /etc/dropbear. By default it points
to /var/tmp, which is tmpfs, i.e. the host keys are deleted at each reboot.

To use permanent host keys:
mkdir -p /var/media/ftp/.dropbear
chmod 700 /var/media/ftp/.dropbear
rm -f /var/tmp/dropbear
ln -s /var/media/ftp/.dropbear /var/tmp/dropbear

Then, start dropbear:
    /usr/local/sbin/dropbear -R -F -E -B
or in background
    /usr/local/sbin/dropbear -R -E -B

This is still very raw. Login shells don't get a proper tty,
passwords dont work.

To connect anyway:
ssh root@192.168.178.1 /bin/sh


TODO
====
- full dropbear integration 
    - Get a proper pty
    - Start via inetd
- IPv6 native (untested, patched lua to enable selection)
- /var/flash/debug.cfg 


HISTORY
=======

release 3
---------------
- Added dropbear binaries (very rudimentary)
- Start telnetd on atom core
- patch to enable native IPV6 selection

release 2 [fb6490_6.50_telnet-2.tar]
------------------------------------
- Changed etc/init.d/rc.tail.sh
	- Explicitly start telnetd from here (obviously the existence of /usr/sbin/telnetd is not always sufficient)
- Added unsquashfs4_avm_x86/mksquashfs4-lzma-avm-be binaries (from Freetz)
- Fixed html tags in README
- Remove LCR download in telnet-1.tar

release 1 [fb6490_6.50_telnet.tar]
----------------------------------
- Initial release

