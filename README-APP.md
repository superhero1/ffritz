Introduction 
============
This image provides some additional software packages for the atom core,
which i have implemented/ported mainly for the purpose of operating the
FritzBox as media player for my dumb old amplifier.

Features
========

All services provided by the application package are invoked at startup
via rc scripts in 

	/nvram/ffnvram/etc/rc.d

which in turn are symlinks to 

	/nvram/ffnvram/etc/init.d

The service name is preceded by two digits controlling the initialization
order.

To (de)activate them just add(remove) symlinks in rc.d.

The service name is preceded by two digits controlling the initialization order.

The tool "ffservice" provides a simple script to operate services:

	ffservice start _servicename_
	ffservice stop _servicename_
	ffservice restart _servicename_
	ffservice enable _servicename_
	ffservice disable _servicename_
	ffservice list

Most user editable configuration files for these services are located
below

	/var/media/ftp/ffritz

or

	/nvram/ffnvram

The script /usr/local/etc/ffshutdown attempts to unmount the application
package by

- stopping all services
- killing all remaining PIDs that access the image
- unmounting /usr/local

ffdeamon
--------

A simple deamon manager is provided with ffdaemon.

Usage: ffdaemon [daemon-args] executable [executable-args]

daemon-args are:

       -n              : No daemon mode
       -C              : Do not close file descriptors
       -r user[:group] : run as user[:group]
       -i secs         : Restart delay after program terminates
       -l loops        : Number of loops to run (0 = default = endless)
       -N name         : Name service rather than using the executable name
       -L              : List all services
       -K name         : Kill named service (%all for all)
       -R name         : Restart named service (%all for all)
       -o dir          : Run service after chroot to dir

Following is a list of services (service name in brackets).

Buildroot environment (buildroot)
---------------------------------

The application package contains the complete buildroot root-filesystem
that was used to build the ffritz application package in /usr/local/buildroot.

The buildroot service will prepare a fully operational filesystem
below /tmp/br. All required special filesystems are mounted (dev, sys, proc).

To use this filesystem, just use chroot:

        chroot /tmp/br

or just "br" as an alias.

There are two flavours for the constructed filesystem:

1. Keeping most of the filesystem read-only
2. Using a writeable overlay for the whole filesystem

Option (1):

-	A overlay is mounted over parts the filesystem using
	unionfs (nvram for /etc, /root, ramdisc for /var) so that the
        filesystem is writeable there.
	Since the nvram overlay is located in /etc/ffnvram/buildroot
	any change made there is persistent.

-	NOTE: The nvram overlay is meant to store configuration files only.
	Bigger data (incl. log files) would easily exceed the available 
	size (which is ca. 5MB)!
	The same applies to /var, where changes reside in volatile RAM.

Option (2):

-	Create/edit the file /nvram/ffnvram/ffbuildroot.conf
-	Add the line BR_USER_OVERLAY=/var/media/ftp/my-root
-	This option will use /var/media/ftp/my-root to store
	new or changed contents of the buildroot filesystem
	in a copy-on-write manner.
-	Preferably, the storage should be located on a mounted
	USB device.
-	NOTE: Be aware that storing files below /var/media/ftp
	exposes them via the NAS service of the box, if enabled!!

Option 1 is a lightweight mechanism to just start some tools or services that
do not require write access to /usr or other locations which are read-only.

Option 2 is useful when more sophisticated tools shall be used, e.g. when
you want to use python or node.js and add your own modules.

All binaries and libraries from the buildroot directory (usr/bin,
usr/lib) are available to FritzOS via symlinks in /usr/local/bin,
/usr/local/lib.
Specific tools and services might require executing them in the chroot
environment.

For example, to start a http server on port 81:

- run "make atom-brconfig" and add lighttpd
- run "make package-atom" to rebuild the application package
- install it to the box as described (ffinstall -r package checksum)
- edit /tmp/br/etc/lighttpd/lighttpd.conf and set the port to 81
- edit /tmp/br/var/www/index.html
- Start the http server: br /etc/init.d/S50lighttpd start

User space player for USB DACs (usbplayd)
-----------------------------------------
- Accepts inputs from mpd, shairport-sync and bluetooth.
- Details in AUDIO.md

Music Player Daemon (mpd)
-------------------------
- Uses user space audio tool (via libusb/libmaru) to access an USB audio DAC
- Additional services are
	- upmpdcli (UPNP/DLNA renderer) 
	- ympd (http frontend at port 82) 
- Refer to AUDIO.md for details

ShairPort Daemon (shairport-sync)
---------------------------------
- Acts as AirPort receiver
- Refer to AUDIO.txt for details

Bluetooth a2dp sink (bluetooth)
------------------------------------
- Reports itself as "FritzBox"
- Tested with Logitech BT stick (CSR chipset)
- Refer to AUDIO.txt for details

nfs mounter
-----------
The file /var/media/ftp/ffritz/.mtab exists can be created to mount specific
nfs directories to an (existing) location below /var/media/ftp.

The format of the file is an nfs URL, options are those accepted by the
fuse-nfs tool:

	MOUNT  nfs://server/service mount-options

For example, to mount the music database from an external NAS:

	MOUNT Musik/NAS nfs://nas/Multimedia/Music --allow_other

nfs server (nfs)
----------------
The "nfs" service starts a user-space nfsv3 service (unfsd). To use it, 
create an exports file in /nvram/ffnvram/etc/exports and run/enable the 
service.
A sample exports file can be found in /usr/local/etc/exports.

Refer to /usr/local/etc/exports for details.

lirc (lircd)
------------
lirc can be used to operate an IR transceiver connected to the fritzbox
(i am using an irdroid module).

- General configuration settings (used driver, network port, ...) can be
  modified in

	/var/media/ftp/ffritz/lirc_options.conf

- Remote control configuration can be placed into

	/var/media/ftp/ffritz/etc/lirc/lircd.conf.d

- To restart lirc after doing this:

	ffdaemon -R lircd

- For irdroid/irtoy the cdc-acm kernel module is packaged and installed.

OpenVPN (openvpn)
-----------------

See OPENVPN.md

Wireguard (wireguard)
---------------------

Experimental. I have observed that the box sometimes hangs when 
removing a wg interface or removing the kernel module.

The wireguard service 

- installs the wireguard.ko kernel module
- create a wireguard interface using the configuration file
  /nvram/ffnvram/etc/wireguard/wg0.conf
- installs an IPv4 UDP forwarding rule for port 51820

Building the application image
==============================

You can either use the pre-built images from

- the download section: https://bitbucket.org/fesc2000/ffritz/downloads
- my private server with dailiy builds (sorry, only ftp for now):
  ftp://ftp.ffesh.de/pub/ffritz/FritzOS7/daily/
- or build it by yourself:

	make package-atom

This will rebuild the image in packages/x86/ffritz.

At the moment there are no configuration options, but you can configure the
contents of the buildroot package.

	make atom-brconfig

This will call menuconfig for the buildroot package, and store the user configuration 
in packages/x86/buildroot/user_defconfig. A subsequent "make package-atom" will apply
the changes to the application image package.
If successfull the new features are available in the application image.

Installation
============

The image is distributed as ffritz-app-puma7-VERSION-fos7.tar. To install it,

- Copy it to the box NAS directory 

	scp image.tar root@192.168.178.1:/var/media/ftp

- Log in and install it

	ffinstall image.tar CHECKSUM

- OR install, terminate old and start new image/services:

	ffinstall -r image.tar CHECKSUM

  The checksum (it is NOT the checksum of the tar file!) is the sha256sum
  listed on the download page, or generated by the build
  (packages/x86/ffritz/image.sha256sum). 
  It is also contained in the file ffimage.sha256sum within the release
  .tar archive.

Steps performed by the startup script:

- check if /var/media/ftp/ffritz/data/ffimage.bin exists
- compare its SHA256 checksum against the checksum that was given at installation.
- if it matches, ffimage.bin is mounted to /usr/local
- The target checksum is saved in the encrypted persistent storage.
- Execute /usr/local/etc/ff_atom_startup

Restart services without box reboot
-----------------------------------
If you do not want to restart the box after installing a new image:

- Stop all ffritz services:

	/usr/local/etc/ffshutdown

- Run mount script for new image: /etc/init.d/S93-ffimage
- Start services: /etc/init.d/S94-ffstart

Current core image supports -r switch as first parameter, which does all this:

	ffinstall -r image.tar CHECKSUM

If ffshutdown fails to unmount /usr/local:

- Try to localize processes still using /usr/local and terminate them
- Terminate all login sessions, log in again and retry
- If all fails, reboot the box
