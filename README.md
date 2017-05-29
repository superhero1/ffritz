Introduction 
============
This is a repository containing my modifications for FritzBox-6490 Cable.
You still need the original image to build a modified install image, and you
need a way to upgrade.  I'm assuming you already have telnet/ssh access to
the box by either already running a modified image or having gained access.

It _might_ work on the FritzBox 6590, but i have never tested it.

There are some known methods how to perform an initial firmware update
from a box that runs an original image.
The one known to work on current firmware versions is to directly write 
kernel and filesystem images via the boot loader (eva). Search IPPF threads
for information on how to accomplish this.

I take no responsibility for broken devices or other problems (e.g. with
your provider).  Use this at your own risk.

If you don't trust my binaries, everything (hopefully) that is required to
rebuild them is located below packages.

Usage
=====

Creating an install/update image
--------------------------------

- Obtain original install image (default is
    FRITZ.Box_6490_Cable.de-en-es-it-fr-pl.141.06.83.image)

- Clone repository (master branch) in the directory where the original
    install image is located:

    `git clone https://fesc2000@bitbucket.org/fesc2000/ffritz.git`

- Go to ffritz directory and `make` (sudo required).

Installing the image (when running firmware < 6.8x)
---------------------------------------------------

- Copy the release tar image to the box, e.g. NAS (/var/media/ftp)
- Extract in / directory of arm core:
    tar xf /var/media/ftp/fb6490_XXX.tar
- Call var/install (from the / directory !!)
	- Monitor the output on the console (1st telnet/ssh login session).
	  There should be a SUCCESS message at the end.
	- The return code $? of var/install should be 1
- After successful upgrade, execute "nohup sh var/post_install&"

Installing the image (when running firmware >= 6.8x)
----------------------------------------------------
- Copy the release tar image to the box, e.g. NAS (/var/media/ftp)
- From there, copy it to the arm core: 
    scp /var/media/ftp/fb6490_XXX.tar root@169.254.1.2:/tmp
- Log in to the arm core:
    ssh root@169.254.1.2
- Extract in / directory of arm core:
    tar xf /tmp/fb6490_XXX.tar
- Call var/install (from the / directory !!)
	- Monitor the output on the console (1st telnet/ssh login session).
	  There should be a SUCCESS message at the end.
	- The return code $? of var/install should be 1
- After successful upgrade, execute "nohup sh var/post_install&"

Installation issues
-------------------

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

- Store password persistently
    ffsync

- Assign a new WEB GUI password

The password is stored persistently in the box'es NVRAM, and is valid for
both arm and atoms ssh service.

Features
========

telnet (arm core feature)
-------------------------

- telnetd is available on the ARM CPU for 5 minutes after startup, then it's
    killed
- telnetd on atom CPU can be started via /usr/sbin/start_atom_telnetd from ARM

dropber/ssh/scp (arm core feature)
----------------------------------
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

dropbear/ssh/scp (Atom core feature)
------------------------------------
- The atom core has no direct access to the NVRAM
- All non-volatile data is passed from arm to atom on startup:
	- host keys for atom are stored in `/nvram/dropbear_x86`. They are
	  generated on the ARM prior to starting the server on atom if they
	  don't exist (in the `ff_atom_startup` script).
	  The data is copied to atom:/var/tmp/dropbear at box startup.

	- /.ssh
	  This is a symlink to /var/tmp/root-ssh, which is populated with the
	  contents of /nvram/root-ssh_x86 at startup. This means:
		- All unsaved runtime data in ~root/.ssh gets lost at reboot
		- If public keys are added to `authorized_keys`, they should be
		  saved to the arm nvram:

		  `scp /var/tmp/root-ssh/authorized_keys root@fritz.box:/nvram/root-ssh_x86`

	- passwords (/etc/shadow) are copied from arm to atom at startup.
	  Changing passwords locally on the atom is not persistent.
	- Startup can be inhibited by creating file
	  `/var/media/ftp/.skip_dropbear`

openssl (Atom core feature)
---------------------------
Nothing to say here.

IPv6 (arm core feature)
-----------------------
For firmware < 6.63 selection of native IPv6 is forced to be enabled in
the GUI together with the general IPv6 availability.

Music Player Daemon (Atom package)
----------------------------------
- Uses user space audio tool (via libusb/libmaru) to access an USB audio DAC
- Refer to MPD.md for details
- Startup can be inhibited by creating /var/media/ftp/.skip_mpd

ShairPort Daemon (Atom package)
-------------------------------
- Acts as AirPort receiver
- Refer to MPD.txt for details
- Startup can be inhibited by creating /var/media/ftp/.skip_shairport

nfs mounter (Atom package)
--------------------------
The file /var/media/ftp/ffritz/.mtab exists can be created to mount specific
nfs directories to an (existing) location below /var/media/ftp.

The format of the file is:

    MOUNT mountpoint mount-options

For example, to mount the music database from an external NAS:

    MOUNT Musik/NAS -o soft nas:Multimedia/Music

lirc (Atom Package)
-------------------
lirc can be used to operate an IR transceiver connected to the fritzbox
(im using an irdroid module).

- General configuration settings (used driver, network port, ...) can be
  modified in

    /var/media/ftp/ffritz/lirc_options.conf

- Remote control configuration can be placed into

    /var/media/ftp/ffritz/etc/lirc/lircd.conf.d

- To restart lirc after doing this:

	killall lircd

- For irdroid/irtoy the cdc-acm kernel module is packaged and installed.
  It is pre-built, but can be generated in packages/x86/avm
  (make kernel-config kernel-modules)

- lircd execution can be prevented by creating /var/media/ftp/.skip_lircd

athtool/pswtool: Switch tools(ARM package)
------------------------------------------
Some tools i have written to access the external switch (AR8327) and 
internal switch (in puma6): athtool and pswtool respectively.

Supported features:

- Reading and writing registers (athtool)
- Configuring port mirroring (athtool)
- Configuring VLANs (athtool, pswtool)
- Reading port counters (athtool, pswtool)

In general, -h shows detailed help.

A topology diagram that shows the usage of these two switches
is outlined in [MISC.md](MISC.html).

NOTES:

- athtool operates the switch on register level. Any modifications done
  are not known by upper layer box services and therefore might cause
  unwanted side-effects (or get overwritten at some time).
- Register access is protected by a semaphore (IPC key 0x61010760).
  Should an application holding this semaphore crash, the semaphore is not
  released and tools will hang endlessly (in fact they can't even be started,
  since the loader of libticc.so attempts to get the semaphore).
  Therefore i packaged the mdio-relese command, which forces a release operation
  on the semaphore.
- pswtool is based on some API calls provided by libticc. It is therefore 
  limited to the few calls i could more or less re-engineer.

OpenVPN
-------

See OPENVPN.md

Miscellaneous tools (Atom/Arm packages)
---------------------------------------
- ldd
- su
- strace
- tcpdump
- tcpreplay
- mpc
- curl
- rsync
- socat

Software Packages
=================

In addition to the core features it is possible to install an application image
to the atom core (see Atom Package features listed above).

The image is distributed as ffritz-x86-VERSION.tar.gz. To install it,

- Copy it to the box NAS directory 

	scp ffritz-x86-VERSION.tar.gz root@192.168.178.1:/var/media/ftp

- Log in and install it

	ffinstall ffritz-x86-VERSION.tar.gz CHECKSUM

  The checksum is the sha256sum listed on the download page. It is also contained
  in the file ffimage.sha256sum within the release .tar.gz archive.

- After the success message, restart the box.


Steps performed by the startup script:
- check if /var/media/ftp/ffritz/data/ffimage.bin exists
- compare its SHA256 checksum against the checksum that was given at installation.
- if it matches, ffimage.bin is mounted to /usr/local
- The target checksum is kept in an encrypted persistent storage.

Notes
=====

Atom libraries
--------------

- Espcially mpd requires a lot of additional shared libraries. Rather than
    integrating them into /lib / /usr/lib, they remain in their own lib
    directory (/usr/local/lib).
    Also, the systems's `LD_LIBRARY_PATH` is not modified. This is to avoid any
    conflicts/incompatibilies with other box services.

    In order to be able to call these binaries they are invoked via a wrapper
    script (bin/exec/ffwrap) which sets `LD_LIBRARY_PATH` before actually
    calling the binary.

Toolchain
---------
Cross-compile toolchain for ARM is buildroot-2013.02 from the original avm
source tarball.
It is installed in packages/arm/avm (just do a make there).

For Atom, the toolchain is buildroot-2013.02 (packages/x86/buildroot).
The atom source tarball (packages/x86/avm) does not work for me.

Build Host
----------

Tested: Debian 7, Debian 8, CentOS 7.

Used disk space is ca. 10G.

Required (debian) packages are:
squashfs-tools busybox rsync sudo gcc g++ flex bison git libncurses-dev gettext unzip automake

Big endian squashfs tools
-------------------------

Binaries are provided in the "hosts" directory. If they dont work, try cloning
freetz and build them using "make squashfstools-be".

Required (debian) packages are:
apt-get install gawk libtool realpath pkg-config zlibc gnulib libcap-dev

You might have to remove "composite" and "sys/acl.h" from the
.build-prerequisites file

TODO / Known Issues
===================
- Fix usbplayd
	- Fix libmaru to properly support for different sample rates
	  (currently only 48KHz is detected)
