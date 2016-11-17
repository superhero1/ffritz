This is a repository containing my modifications for FritzBox-6490 Cable.
You still need the original image to build a modified install image, and you
need a way to upgrade.  I'm assuming you already have telnet/ssh access to
the box by either already running a modified image or having gained access
(see INSTALL.txt).

I take no responsibility for broken devices or other problems (e.g. with
your provider).  Use this at your own risk.

If you don't trust my binaries, everything (hopefully) that is required to
rebuild them is located below packages.

USAGE
=====

Creating an install/update image
--------------------------------
- Obtain original install image (default is
    FRITZ.Box_6490_Cable.de-en-es-it-fr-pl.141.06.62.image)

- Clone repository (master branch) in the directory where the original
    install image is located:

    git clone https://fesc2000@bitbucket.org/fesc2000/ffritz.git

- Decide whether the atom filesystem shall be modified or not
    (see "Atom core extensions" below)

Installing the image
--------------------
- Go to ffritz directory and "make release" (sudo required).
  This will create a release directory with a tar file inside, which can be
  used for upgrade:
	- Copy to box, e.g. NAS (/var/media/ftp)
	- Extract in / directory of arm core:
	    tar xf /var/media/ftp/fb6490_XXX.tar
	- Call var/install (from the / directory !!)
	    - Monitor the output on the console (1st telnet/ssh login session).
		There should be a SUCCESS message at the end.
	    - The return code $? of var/install should be 1
	- After successful upgrade, execute "nohup sh var/post_install&"


Installation issues:
    If the return code is not 1, check the console messages. Known errors are

    - ARM Calling additional procedure failed: UPDATE FAIL (1)
	It is likely that there was a problem with the certificate update,
	probably because the box does not have current certificates and new
	certificates could not be obtained (missing network connection?).

	The image has been installed, but the boot bank has not been switched
	(i.e.  the box will still boot the current image).
	To switch to the new image anyway, execute var/switch_image and apply
	the instructed command.

    - Installer not called from root directory (then the "dd" command can't
	be found)

First use
---------
After first installation a ssh login password must be assigned (if you plan to
use ssh).
Since you need telnet for this, and all traffic of the telnet session can
theoretically be sniffed, you should re-assign passwords after the session
and never use telnet any more:

- telnet to the box:
    telnet 192.168.178.1
    Password is the WEB GUI login.

- Assign a password for root
    passwd

- Log out of the telnet session

- ssh to the box (using the new password):
    ssh root@192.168.178.1

- Assign a different password once more:
    passwd

- If you run ssh on the atom core, copy passwords there:
    cp /nvram/shadow /var/remote/var/tmp/shadow

- Assign a new WEB GUI password

The password is stored persistently in the box'es NVRAM, and is valid for
both arm and atoms ssh service.

Note: The arm core is usually accessible at x.x.x.1, the atom core at x.x.x.254

Atom core extensions
--------------------
There is a separate package (ffritz-x86-ver.tar.gz) that contains various
extensions for the atom core (see below).
The package is not part of the git repository and needs to be downloaded from
https://bitbucket.org/fesc2000/ffritz/downloads and put to .. directory.

These extensions can either be hardcoded into the atom filesystem or installed
to the NAS directory (/var/media/ftp) later by the user.
The latter is more flexible, but has some drawbacks:

- IT IS INSECURE!
  There are various scripts/binaries that are (have to be) executed as root.
  If someone has access to the box NAS storage he can modify these
  binaries/scripts and do evil things.

- The provided services are not automatically started when the box restarts.

To generate an install image that has the modifications in the Atom root
filesystem (/usr/local):
    Make sure FFRITZ_X86_PACKAGE is defined in the top-level Makefile


To install the package to the box NAS (/var/media/ftp) without having it in
the filesystem:
    - Copy packages/x86/ffritz/ffritz-x86-VERSION.tar.gz to the NAS storage
	scp packages/x86/ffritz/ffritz-x86-VERSION.tar.gz root@192.168.178.1:/var/media/ftp
    - Log in to the arm core:
	cd /var/media/ftp
	gunzip -c ffritz.tar.gz | tar xf -

    The services then need to be started manually:
	/var/media/ftp/ffritz/etc/ff_atom_startup

    This script will run the necessary steps on both the arm and atom core.

FEATURES
========

telnet
------
- telnetd is available on the ARM CPU for 5 minutes after startup, then it's
    killed
- telnetd on atom CPU can be started via /usr/sbin/start_atom_telnetd from ARM

dropber/ssh/scp (arm)
------------------
- By default, root has no password, and other users do not have rights to get
    a tty, so no login possible by default.
- A root password can be assigned via a telnet login within 5 mins after
    bootup (passwd).
- The system startup script makes sure it's persistent by storing /etc/shadow
    in /nvram, so future updates do not require setting the password again
- The box host keys are created on demand (first ssh login) and stored in
  /nvram/dropbear
- The roots .ssh directory is a symlink to
  /nvram/root-ssh

dropbear/ssh/scp (Atom)
-----------------------
- The atom core has no direct access to the NVRAM
- All non-volatile data is passed from arm to atom on startup:
    - host keys for atom are stored in /nvram/dropbear_x86. They are generated
      on the ARM 
      prior to starting the server on atom if they don't exist
      (in the ff_atom_startup script).
      The data is copied to atom:/var/tmp/dropbear at box startup.

    - /.ssh
      This is a symlink to /var/tmp/root-ssh, which is populated with the
      contents of /nvram/root-ssh_x86 at startup. This means:
	- All unsaved runtime data in ~root/.ssh gets lost at reboot
	- If public keys are added to authorized_keys, they should be saved to
	  the arm nvram:
	  scp /var/tmp/root-ssh/authorized_keys root@fritz.box:/nvram/root-ssh_x86
    - passwords (/etc/shadow) are copied from arm to atom at startup. Changing
      passwords locally on the atom is not persistent.
    - Startup can be inhibited by creating file /var/media/ftp/.skip_dropbear

IPV6
----
Selection of native IPv6 has been forced to be enabled in the GUI together with
the general IPv6 availability.

This is obsolete with firmware 6.63, but doesn't harm.

Music Player Daemon (Atom)
-----------------------------
- Uses user space audio tool (via libusb/libmaru) to access an USB audio DAC
- Refer to MPD.txt for details
- Startup can be inhibited by creating /var/media/ftp/.skip_mpd

ShairPort Daemon (Atom)
-----------------------
- Acts as AirPort receiver
- Refer to MPD.txt for details
- Startup can be inhibited by creating /var/media/ftp/.skip_shairport

nfs mounter (Atom)
------------------
The file /var/media/ftp/ffritz/.mtab exists can be created to mount specific
nfs directories to an (existing) location below /var/media/ftp.

The format of the file is:
MOUNT mountpoint mount-options

For example, to mount the music database from an external NAS:

MOUNT Musik/NAS -o soft nas:Multimedia/Music

Miscellaneous tools (Atom)
--------------------------
- ldd
- su
- strace
- tcpdump
- tcpreplay
- mpc

NOTES
=====

Atom libraries
--------------
- Espcially mpd requires a lot of additional shared libraries. Rather than
    integrating them into /lib / /usr/lib, they remain in their own lib
    directory (/usr/local/lib).
    Also, the systems's LD_LIBRARY_PATH is not modified. This is to avoid any
    conflicts/incompatibilies with other box services.

    In order to be able to call these binaries they are invoked via a wrapper
    script (bin/exec/ffwrap) which sets LD_LIBRARY_PATH before actually calling
    the binary.

Toolchain
---------
Cross-compile toolchain for ARM is buildroot-2013.02 from the original avm
source tarball.
It is installed in packages/arm/avm (just do a make there).

For Atom, the toolchain is buildroot-2013.02 (packages/x86/buildroot).
The atom source tarball (packages/x86/avm) does not work for me.

Build Host
----------

My build host is Debian 8.2 / x86_64.
Compiling the atom toolchain requires gcc-4.7 installed.

Required packages are:
    squashfs-tools
    busybox
    rsync
    sudo


TODO
====
- Atom: Add ssh as symlink to dbclient
- Atom: Log dropbear messages (stderr) to console
- ARM: Add own stuff in /nvram to /etc/docsis/nvramdontremove to avoid it 
    being removed after recovery

HISTORY
=======

release 9
---------
- Atom
    - Fix access rights to /var/tmp/volume file at startup
    - Make usbplayd self-respawning if it crashes
    - Don't log usbplayd to /var/tmp to avoid hogging ramfs space
    - Forward dropbear stderr outputs to /dev/console
- ARM
    - Make sure that ssh stuff and passwords in /nvram are not cleared
      when a factory reset is performed (by means of entries in 
      /etc/docsis/nvramdontremove). Only for FW 6.6x and later.
    - Do not apply ipv6 patch for 6.63 and later
- Make sure directory permissions for /var/media/ftp/ffritz are correct

release 8
---------
- tested with firmware 6.63
- Atom
    - Added udev rule to give usb devices proper permissions (usb group)
	Remove clumsy permission fixup in runmpd
    - Put ffritz user into usb group by default
    - Fixed permission of mpd.conf
    - Added some tools:
	strace
	ldd
	tcpdump
	tcpreplay
	su
	mpc
    - Replaced usbplay with usbplayd, which is fifo based and accepts several
	inputs. It also performs sample rate conversion if required.
    - Accordingly, mpd uses fifo output driver instead of pipe driver
    - Added shairport (AirPort audio receiver)
    - Added user mount table (var/media/ftp/ffritz/.mtab)
- ARM
    - Added /usr/local/etc/switch_bootbank script to help bank switch without
	having to invoke the bootloader.
	Also in installer tar: var/switch_bootbank
    - Use toolchain from avm source tarball for dropbear binaries

release 7
---------
- Atom
    - Add dropbear
    - Add mpd
- Arm
    - Add startup script /usr/local/etc/init.d/ff_atom_startup

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
	- Explicitly start telnetd from here (obviously the existence of
	  /usr/sbin/telnetd is not always sufficient)
- Added unsquashfs4_avm_x86/mksquashfs4-lzma-avm-be binaries (from Freetz)
- Fixed html tags in README
- Remove LCR download in telnet-1.tar

release 1 [fb6490_6.50_telnet.tar]
----------------------------------
- Initial release

Standalone Packages
===================

ffritz-arm-XXX.tar.gz
---------------------
    - 0.1
	- Created, to be installed to /var/media/ftp/ffritz-arm
	  Contains some libs, tcpreplay, dump, ..., strace, ldd
    
ffritz-x86-XXX.tar.gz
---------------------
    - Version >= 9
	- Aligned versioning with repository version
    - Version 0.4
	- Corresponds to release 8
