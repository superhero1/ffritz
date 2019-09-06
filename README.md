!!!!!!

This is the branch for FritzBox 6591, which is under development. 


INSTALLATION ON 6591
====================

Initial setup:
--------------
- Find some way to get shell access. There is no known method so far.
- Get the latest firmware update image (named FRITZ.Box_6591_Cable ...).
  Make sure that the ORIG definition in Makefile points to it
- A simple make should now generate a release image containing basic tools
  for login (as for 6x90).

Update from shell
-----------------
- **
- ** WARNING:
- **  THERE IS NO KNOWN MATHOD TO RECOVER FROM BOOTLOADER/EVA 
- **  ONCE YOU HAVE BRICKED YOUR BOX 
- **
- Copy the release image to NAS folder and run these commands:
~~~
        cd /var/media/ftp
        tar xf fb6591.....tar
        cp var/switch_bootbank /tmp
        /tmp/switch_bootbank
~~~
- Sample output:
~~~
        SELECTED boot bank        1
        RUNNING firmware version: 161.07.04  [/dev/mmcblk0p9]  modified
        BACKUP firmware version:  161.07.04  [/dev/mmcblk0p3]
~~~
- The new image will be written to the BACKUP partitions. If this is not
  what you want, switch the boot bank and reboot (see below).
- Install the image.
~~~
        /sbin/burnuimg var/firmware-update.uimg || echo FAILED
~~~
- Switch bootbank and reboot the box if the previous step was sucessfull
~~~
        /bin/aicmd pumaglued uimg switchandreboot
~~~
- _ALWAYS_ make sure that there is one known image (0 or 1) which boots, and
  don't overwrite it!


Descriptions further below may or may not apply (unchanged from 6x90)
 
Introduction 
============
This is a repository containing my modifications for FritzBox-6490 Cable.
You still need the original image to build a modified install image, and you
need a way to upgrade.  I'm assuming you already have telnet/ssh access to
the box by either already running a modified image or having gained access.

It is known to work on 6590 as well, although i do not test it.

There are some known methods how to perform an initial firmware update
from a box that runs an original image.
The one known to work on current firmware versions is to directly write 
kernel and filesystem images via the boot loader (eva). 
See 'Installing the image from the Boot Loader' below for information.

I take no responsibility for broken devices or other problems (e.g. with
your provider).  Use this at your own risk.

If you don't trust my binaries, everything (hopefully) that is required to
rebuild them is located below packages.

Have Fun,
<f/e/s/c/2/0/0/0/@/g/m/a/i/l/./c/o/m>

NOTE: This is the main branch which only supports FrizOS 7. Older versions
      are kept on the "fritzos6" branch.

Usage
=====

The modification consists of two parts:
1. Adding basic services (like ssh) to the AVM firmware.
2. An additional application package, consisting of various system tools,
   but is mainly intended to use the Box as audio player.

This README covers the first topic. For information on the application
image see README-APP.md.

Creating an install/update firmware image
-----------------------------------------

- Clone repository (master branch) in the directory where the original
  install image is located:

    `git clone https://fesc2000@bitbucket.org/fesc2000/ffritz.git`

  For FritzOS 6 use:

    `git clone https://fesc2000@bitbucket.org/fesc2000/ffritz.git -b fritzos6`

- Go to ffritz directory and run "make".

Note that this will use pre-built binaries checked into the git repository.
To rebuild these binaries:

	make rebuild

If you want to build an image based on a different original firmware, select
the URL definition in the Makefile (by commenting out the one you want).
Or redeine ORIG to point to the image file (e.g. for Labor images).

The same applies for building an image for FritzBox 6590, edit the Makefile.

IMPORTANT NOTE
--------------

Before starting to modify the Box it is always recommended
to generate (extended) support data (<http://192.168.178.1/support.lua>)
and save it. It might become (very) useful to "unbrick" your Box ...

Installing the image from the Boot Loader (if box has no telnet/ssh login)
--------------------------------------------------------------------------

Please consult the web/IPPF threads. A very good HOWTO is:

https://www.ip-forum.eu/howto-aendern-des-branding-und-installieren-der-retail-firmware-bei-fritz-box-cable-160

Installing the image (with ssh/telnet access)
---------------------------------------------

- Copy the release tar image to the box, e.g. NAS (/var/media/ftp)
- Log in to primary IP address (default 192.168.178.1), which is the ARM
  core on older firmware and Atom on Firmware 6.8 onwards.
- Extract in the root directory:

~~~
	cd /
	tar xf /var/media/ftp/fb6490_XXX.tar
~~~

- Call "var/install" (from the root directory!)
	- Monitor the output on the console (1st telnet/ssh login session).
	  There should be a SUCCESS message at the end.
	- The return code $? of var/install should be 1
- After successful installation, execute "/sbin/reboot".

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

After first installation, ssh login needs to be set up:

- Log in to the box using telnet and the web password within the first 10
  minutes after startup.
- For password authentication, call "passwd" to change the root password
- For pubkey authentication, put public keys to /.ssh/authorized_keys
- Call "nvsync" to make the change persistent

Firmware 6.8 onwards:
To be able to log in to the arm core (address 169.254.1.2 from atom) you might
want to copy the shadow file to persistent storage on arm core:

	rpc sh -c "rpc_cp /etc/shadow /nvram/shadow"

Features
========

Persistent storage
------------------
Various data is stored persistently in an encrypted tar archive
(/var/media/ftp/ffritz/data/ffstore.dat). The password to this
storage is also stored here persistenty (key.enc), but it is
encrypted.

ffstore.dat is basically a tar image of everything below /tmp/ffnvram.
By default this is

- the password database (/etc/shadow file)
- dropbear data (dropbear directory)
- the roots .ssh directory (root_ssh directory), which also contains a subfolder
  with the data for OpenVPN (root_ssh/openvpn).

The nvsync tool re-generates the persistent storage with the contents of 
/tmp/ffnvram.

The executable script /tmp/ffnvram/etc/rc.user, if it exists, is executed at
the end of system startup.

telnet (Atom)
-------------
- A telnetd service is started at boot time and killed after 600 seconds.

dropber/ssh/scp (Atom)
----------------------
- The dropbear host key (/var/tmp/dropbear/dropbear_rsa_host_key) is always
  overwritten with the data derived from the web/SSL key of the box
  (/var/flash/websrv_ssl_key.pem).
- In order to use a different box key, create the file
  /var/tmp/dropbear/rsa_key_dontoverwrite and create a different rsa key
  using dropbearkey.
  Don't forget to save the chages using nvsync.

dropbear/ssh/scp (Arm)
----------------------
- There is a dropbear service running on arm core which fetches it's persistent
  data from /nvram. This might be dropped in future.
- If no password is assigned, use the rpc command to transfer the shadow file
  from atom to arm

    rpc sh -c "rpc_cp /etc/shadow /nvram/shadow"

openssl (Atom)
---------------------------
- /usr/bin/openssl is a wrapper to make sure openssl uses the correct libraries.

pswtool: Switch tools (Arm)
---------------------------
A tool i have written to access the internal switch (in the Puma6 SoC). 

Supported features:

- Configuring VLANs (pswtool)
- Reading port counters (pswtool)

In general, -h shows detailed help.

A topology diagram that shows the usage of these two switches
is outlined in [MISC.md](MISC.html).

NOTES:

- pswtool is based on some API calls provided by libticc. It is therefore 
  limited to the few calls i could more or less re-engineer.

User defined startup script
---------------------------
The file /tmp/ffnvram/etc/init.d/rc.user can be used to add your own startup
commands.

Don't forget to save with "nvstore".

Software Packages
=================

In addition to the core features it is possible to install an application image
to the Atom core (see README-APP.md).

Optional Arm package
--------------------

The file ffritz-arm-VERSION.tar.gz contains some optional tools for the arm core.
It can be integrated into the install image by editing Makefile and defining its
location via the FFRITZ_ARM_PACKAGE variable.

To build this package by yourself:

	make package-arm

Or download it from https://bitbucket.org/fesc2000/ffritz/downloads/ and copy it to
packages/arm/ffritz.

Notes
=====

Toolchain
---------
Cross-compile toolchain for ARM and atom is buildroot-2016.05.
It is installed in packages/(arm|x86)/buildroot (just do a make there).

Build Host
----------

Tested: Debian 7, Debian 8, CentOS 7, Ubuntu 14.04, Ubuntu 16.04 (preferred)

Used disk space is ca. 10G.

Required (debian) packages are:
gcc g++ make bison libreadline-dev gawk libtool realpath pkg-config zlibc gnulib libcap-dev rsync busybox curl wget squashfs-tools flex python perl zip unzip tcl bzip2 locales git xsltproc libncurses-dev gettext sudo bc subversion fakeroot

Big endian squashfs tools
-------------------------

Binaries are provided in the "hosts" directory. If they dont work, try cloning
freetz and build them using "make squashfstools-be".

There is a make rule "squashfstools-be" that does this.

Required (debian) packages are:
apt-get install gawk libtool realpath pkg-config zlibc gnulib libcap-dev

You might have to remove "composite" and "sys/acl.h" from the
.build-prerequisites file

Credits
=======
Thanks to

- PeterPawn for his tools [https://github.com/PeterPawn], ideas and general contributions
- Themaister for libmaru [https://github.com/Themaister/libmaru]
- Bluekitchen for the user space bluetooth stack [https://github.com/bluekitchen]
- Contributors (Flole), testers, ...
- AVM, after all

