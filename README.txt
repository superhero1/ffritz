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
Workaround is to link statically, or to not use ctype.h macros from dropbear ..

Tools
-----
dropbear (dropbear-2016.74/)
    - To start (debug, in chroot):
    /usr/local/sbin/dropbear -R -F -E -B


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

ssh/scp
-------
- By default, root has no password, and other users do not have rights to get a tty, so
  no login possible by default.
- A root password can be given via a telnet login (passwd).
- The system startup script makes sure it's persistent by storing /etc/shadow in /nvram
- The box host keys are created on demand (first ssh login) and stored in
    /var/media/ftp/.dropbear
  Whether this is a good idea i don't know, but keys in volatile RAM are aven 
  worse, and i don't want to clutter /nvram.

TODO
====
- Use box private keys for dropbear
- Use box user management for dropbear 

HISTORY
=======
Next
----
- Store/move dropbear keys to /nvram
- Kill telnetd after 5 minutes
- Do not start telnetd on atom by default
- Remove execution of /var/flash/debug.cfg
- Add ssh as symlink to dbclient
- Add /.ssh as symlink to /var/tmp/root-ssh, and 
  link this to nvram at startup

release 5
---------
- Support for 6.62

release 4
---------
- IPV6 native tested, seems to work
- Better dropbear integration, ssh daemon started by default

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

