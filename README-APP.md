Introduction 
============
This image provides some additional software packages for the atom core,
which i have implemented/ported mainly for the purpose of operating the
FritzBox as media player for my dumb old amplifier.

Features
========

All services provided by the application package are invoked at startup
via rc scripts in 

	/tmp/ffnvram/etc/rc.d

which in turn are symlinks to 

	/tmp/ffnvram/etc/init.d

The service name is preceded by two digits controlling the initialization
order.
To (de)activate them just add(remove) symlinks in rc.d and make the changes
persistent by calling nvsync. The service name is preceded by two digits
controlling the initialization order.

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

The script /usr/local/etc/ffshutdown attempts to unmount the application
package by
- stopping all services
- killing all remaining PIDs that access the image
- umnounting /usr/local

A simple deamon manager is provided with ffdaemon.

       -n : No daemon mode
       -C : Do not close FDs
       -r : run as user[:group]
       -i : Restart delay after program terminates
       -l : Number of loops to run (0 = default = endless)
       -N : Name service rather than using the executable name
       -L : List all services
       -K : Kill named service (%all for all)
       -R : Restart named service (%all for all)
       -o : Run service after chroot to dir

Following is a list of services (service name in brackets).

Buildroot environment (buildroot) - experimental
------------------------------------------------

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

-	A ramdisc overlay is mounted over parts the filesystem using
	unionfs (/etc, /var, /root) so that the filesystem is writeable there.
	Since the ramdisc overlay is located in /etc/ffnvram/buildroot
	any change made there can be made persistent by calling nvsync.

-	NOTE: The ramdisc overlay is meant to store configuration files only.
	Bigger data (incl. log files) would easily exceed the available RAM
	size (which is ca. 100MB).

Option (2):

-	Create/edit the file /tmp/ffnvram/ffbuildroot.conf
-	Add the line BR_USER_OVERLAY=/var/media/ftp/my-root
-	This option will use /var/media/ftp/my-root to store
	new or changed contents of the buildroot filesystem
	in a copy-on-write manner.
-	Preferably, the storage should be located on a mounted
	USB device.
-	Dont forget nvsync to make the config-file persistent.
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
- call nvsync to make configuration persistent
- Start the http server: br /etc/init.d/S50lighttpd start

User space player for USB DACs (usbplayd)
-----------------------------------------
- Accepts inputs from mpd, shairport and bluetooth.
- Details in MPD.md

Music Player Daemon (mpd)
-------------------------
- Uses user space audio tool (via libusb/libmaru) to access an USB audio DAC
- Additional services are
	- upmpdcli (UPNP/DLNA renderer) 
	- ympd (http frontend at port 82) 
- Refer to MPD.md for details

ShairPort Daemon (shairport)
----------------------------
- Acts as AirPort receiver
- Refer to MPD.txt for details

Bluetooth a2dp sink (a2dp_sink_demo)
------------------------------------
- Reports itself as "FritzBox"
- Tested with Logitech BT stick (CSR chipset)

nfs mounter
-----------
The file /var/media/ftp/ffritz/.mtab exists can be created to mount specific
nfs directories to an (existing) location below /var/media/ftp.

The format of the file is an nfs URL, options are those accepted by the
fuse-nfs tool:

	MOUNT  nfs://server/service mount-options

For example, to mount the music database from an external NAS:

	MOUNT Musik/NAS nfs://nas/Multimedia/Music --allow_other

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

Enhanced DVB-C Transport Stream Forwarding (cableinfo)
------------------------------------------------------

THIS IS EXPERIMENTAL FOR FRITZOS 7. In the current state it can crash the box!

To enhance DVB-C streaming performance the DVB-C transport stream can be 
forwarded to an external service which generates the RTP packets.

For this the cableinfo daemon is executed with a wrapper library. For details
refer to packages/x86/libdvbfi/README.txt.

Packet counters
---------------

The pcount tool (on both arm/atom) can watch various packet counters and
calculate rates.

Usage:
~~~~
 --pp-counters|-p [<all>[,<filter>]]               (ARM only!)
 --l2sw-counters|-l <p>[,<all>[,<filter>]]         (ARM only!)
 --netif-counters|-i <p>[,<all>[,<filter>]]
 --extsw-counters|-i <p>[,<all>[,<filter>]]	   (Atom only)
                  : Print counters of port <p> (-1 for all ports).
                    If <all> is 1 all counters are printed, otherwise only
                    those that have changed since the previous call.
                    Use optional <filter> for counter name substring match.
 --reset|-z       : Reset max. rate after output
 --slot|-s <num>  : A storage slot where counter history is kept.
                    Usable for different average times, e.g. slot 0 for
                    fast read and 1 for slow read
 --prtg|-x <mode> : Output in PRTG extended sensor (XML) format:
                    1 : Absolute values
                    2 : Relative values
                    3 : Rate (1/s)
                    4 : Maximum rate (1/s)
                    5 : Both rate and maximum rate
~~~~

- pp-counters:    These are some counters from the packet processor
- l2sw-counters:  Counters of the internal L2 switch
- netif-counters: Counters of all network interfaces (from /proc/net/dev)
- extsw-counters: Counters of external switch ports 

To call the arm tool from atom, just use "rpc pcount ...".

Example output:
~~~~
name/port  counter name Incr. since last call Total count Rate       Max. rate
---------- ------------ --------------------- ----------- ---------- ---------
acc0[1]:   RX_octets  : +608                  549,962,764 1,870/s  < 1,870/s
~~~~

NOTE: The ARM version of the tool needs to be integrated into the firmware update.
      This is done by downloading ffritz-arm-0.6.tar.gz (or later) into
      packages/arm/ffritz before building the firmware image.


PRTG Support
------------

The PRTG Network monitor allows to use tools it runs via ssh login,
expecting XML output. pcount can directly provide this output with
the -x parameter.

These tools need to be located in /var/prtg/scriptsxml. The pcount tool itself
and various wrapper scripts will be placed there at startup.

- datarate_atom     : Reports rates of all atom counters
- pktrate_atom      : Same for packets
- max_datarate_atom : Reports maximum rate observed in 1 sec interval
- max_pktrate_atom  : Same for packets
- (The same sensor scripts exist for ARM. See NOTE above.)
- pcount_arm       : rpc wrapper to invoke pcount on arm

To directly call pcount as sensor tool make sure to put parameters into quotes.
For example when creating a "pcount_arm" sensor:

~~~~
"-p1,MPDSP_frwrd_to_host" "-x2"
~~~~

This will create a sensor that monitors packets forwarded to ARM due to LUT miss,
which is an indication for a DOS attack ;-).

Building application image
==========================

You can either use the pre-built images from the download section
(https://bitbucket.org/fesc2000/ffritz/downloads/) or build it
by yourself:

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

The image is distributed as ffritz-app-VERSION.tar. To install it,

- Copy it to the box NAS directory 

	scp ffritz-app-VERSION.tar root@192.168.178.1:/var/media/ftp

- Log in and install it

	ffinstall /var/media/ftp/ffritz-app-VERSION.tar CHECKSUM

  The checksum is the sha256sum listed on the download page, or
  generated by the build (packages/x86/ffritz/ffritz-app-VERSION.sha256sum). 
  It is also contained in the file ffimage.sha256sum within the release
  .tar archive.

- After the success message, restart the box (or read the next chapter)

Steps performed by the startup script:

- check if /var/media/ftp/ffritz/data/ffimage.bin exists
- compare its SHA256 checksum against the checksum that was given at installation.
- if it matches, ffimage.bin is mounted to /usr/local
- The target checksum is saved in the encrypted persistent storage.
- Execute /usr/local/etc/ff_atom_startup

Restart services without box reboot
-----------------------------------
If you do nott want to restart the box after installing a new image:
- Stop all ffritz services:

	/usr/local/etc/ffshutdown

- Run mount script for new image: /etc/init.d/S93-ffimage
- Start services: /etc/init.d/S94-ffstart

Current core image supports -r switch as first parameter, which does all this:

	ffinstall -r ffritz-app-VERSION.tar CHECKSUM

If ffshutdown fails to unmount /usr/local:

- Try to localize processes still using /usr/local and terminate them
- Terminate all login sessions, log in again and retry
- If all fails, reboot the box

Notes
=====

Atom libraries
--------------

- Espcially mpd requires a lot of additional shared libraries. Rather than
    integrating them into /lib / /usr/lib, they remain in their own lib
    directory (/usr/local/lib).
    Also, the systems `LD_LIBRARY_PATH` is not modified. This is to avoid any
    conflicts/incompatibilies with other box services.

    In order to be able to call these binaries they are invoked via a wrapper
    script (bin/exec/ffwrap) which sets `LD_LIBRARY_PATH` before actually
    calling the binary.
