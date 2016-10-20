This is a repository containing my modifications for FritzBox-6490 Cable.
You still need the original image to build a modified install image, and you need a way to upgrade.
I'm assuming you already have telnet/ssh access to the box by either already running a modified image
or having gained access (see INSTALL.txt).

USAGE
=====
- Clone repo (use the latest release tag)
- Copy original install image (default is FRITZ.Box_6490_Cable.de-en-es-it-fr-pl.141.06.62.image)
  to directory above repo
- Go to repo and "make release" (sudo required). This will create a release direcroy with a tar file inside, which can
  be used for upgrade:
	- Copy to box (e.g. NAS)
	- Extract in / directory (tar xf /var/media/ftp/fb6490_6.tar)
	- Call var/install
	- After successful upgrade, execute "nohup sh var/post_install&"

Atom core extensions
--------------------
By default, only the arm core filesystem is modified. There is an additional package (ffritz-x86-ver.tar.gz) 
that contains various extensions for the atom core (mpd, dropbear at the moment).
These extensions can either be hardcoded into the atom filesystem (edit the Makefile and 
comment out the FFRITZ_X86_PACKAGE). Then they will be located in /usr/local.

Alternatively, the ffritz package can be copied to the box (/var/media/ftp) and extracted there:
gunzip -c ffritz.tar.gz | tar xf -

The services then need to be started manually:
    - mpd only
	On arm, call "rpc /var/media/ftp/ffritz/etc/runmpd"
	
    - dropbear and/or mpd:
	After box startup, run this command on the arm core:

	/var/media/ftp/etc/ff_atom_startup

	This will do all the magic to transfer required data to the atom and start the
	services (see below).

FEATURES
========

telnet
------
- telnetd is available on the ARM CPU for 5 minutes after startup, then it's killed
- telnetd on atom CPU can be started via /usr/sbin/start_atom_telnetd from ARM 

ssh/scp (dropbear)
------------------
- By default, root has no password, and other users do not have rights to get a tty, so
  no login possible by default.
- A root password can be assigned via a telnet login within 5 mins after bootup (passwd).
- The system startup script makes sure it's persistent by storing /etc/shadow in /nvram,
  so future updates do not require setting the password again
- The box host keys are created on demand (first ssh login) and stored in
  /nvram/dropbear
- The roots .ssh directory is a symlink to
  /nvram/root-ssh

dropbear on atom
----------------
- All non-volatile data is passed from arm to atom on startup:
    - host keys for atom are stored in /nvram/dropbear_x86. They are generated on the ARM 
      prior to starting the server on atom if they dont exist (in the ff_atom_startup
      script).
      The data is copied to atom:/var/tmp/dropbear at box startup.

    - /.ssh
      This is a symlink to /var/tmp/root-ssh, which is populated with the contents of
      /nvram/root-ssh_x86 at startup. This means:
	- All unsaved runtime data in ~root/.ssh gets lost at reboot
	- If public keys are added to authorized_keys, they should be saved to the arm
	  nvram:
	  scp /var/tmp/root-ssh/authorized_keys root@fritz.box:/nvram/root-ssh_x86
    - passwords (/etc/shadow) are shared with the arm core.
    - Startup can be inhibited by creating file /var/media/ftp/.skip_dropbear

Music Player Daemon (on atom)
-----------------------------
- Uses user space audio tool (via libusb/libmaru) to access an USB audio DAC
- Refer to MPD.txt for details
- Startup can be inhibited by creating /var/media/ftp/.skip_mpd


IPV6
----
Selection of native IPv6 has been forced to be enabled in the GUI together with the general
IPv6 availability.

FritzBox as audio player (mpd)
------------------------------
Work in progress. See "sound" branch and/or download section on bitbucket

NOTES
=====

Atom libraries
--------------
- Espcially mpd requires a lot of additional shared libraries. Rather than integrating 
    them into /lib / /usr/lib, they remain in their own lib directory (/usr/local/lib).
    Also, the systems's LD_LIBRARY_PATH is not modified. This is to avoid any 
    conflicts/incompatibilies with other box services.

    In order to be able to call these binaries they are invoked via a wrapper
    script (bin/exec/ffwrap) which sets LD_LIBRARY_PATH before actually calling
    the binary.

Toolchain
---------
Cross-compile toolchain for ARM is buildroot-2013.08 (it's the first one i looked for that
matches the uClibc version used in FB firmware 6.50).

There seems to be a problem with locale support, since dropber binaries complain
about missing symbols (caused by usage of ctype.h macros like isalnum, toupper, ...).
Workaround is to link statically, or to not use ctype.h macros from dropbear ..

For Atom, the toolchain is buildroot-2013.02.

Build Host
----------

My build host is Debian 8.2.
Compiling the atom toolchain requires gcc-4.7 installed.


TODO
====
- IPv6 is always disabled after box restart
- Write proper udev rule to adjust dev attributes

HISTORY
=======

release 8
---------
- add udev rule to give usb devices proper permissions (usb group)
    Remove clumsy permission fixup in runmpd
- put ffritz user into usb group by default
- fix permission of mpd.conf
- Add some tools:
    strace
    ldd
    tcpdump
    tcpreplay


release 7
---------
- Add dropbear for atom
- Add mpd to atom

release 6
---------
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

