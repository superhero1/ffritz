This is a repository containing my modifications for FritzBox-6490 Cable.
You still need the original image to build a modified install image, and you need a way to upgrade.
I'm assuming you already have telnet/ssh access to the box by either already running a modified image
or having gained access (see INSTALL.txt).

USAGE
=====
- Clone repo
- Copy original install image (default is FRITZ.Box_6490_Cable.de-en-es-it-fr-pl.141.06.62.image)
  to directory above repo
- Go to repo and "make install" (sudo required). This will create a release direcroy with a tar file inside, which can
  be used for upgrade:
	- Copy to box (e.g. NAS)
	- Extract in / directory (tar xf /var/media/ftp/fb6490_6.tar)
	- Call var/install
	- After successful upgrade, execute "nohup sh var/post_install&"

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


FEATURES
========

telnet
------
- telnetd is available on the ARM CPU for 5 minutes after startup, then it's killed
- telnetd on atom CPU can be started via /usr/sbin/start_atom_telnetd from ARM

ssh/scp
-------
- By default, root has no password, and other users do not have rights to get a tty, so
  no login possible by default.
- A root password can be given via a telnet login (passwd).
- The system startup script makes sure it's persistent by storing /etc/shadow in /nvram
- The box host keys are created on demand (first ssh login) and stored in
  /nvram/dropbear
- The roots .ssh directory is a symlink to
  /nvram/root-ssh

IPV6
----
Selection of native IPv6 has been forced to be enabled in the GUI together with the general
IPv6 availability.

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
- Repack telnet-1.tar with busybox (same for release image)

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

