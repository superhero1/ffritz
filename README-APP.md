aIntroduction 
============
This image provides some additional software packages for the atom core,
which i have implemented/ported mainly for the purpose of operating the
FritzBox as media player for my dumb old amplifier.

If the box fails to start due to some issue in the application image, its 
execution can be prevented by adding the keyword 'ffimage=0' to the 
kernel_args nvram variable in the EVA bootloader:

Log in to the bootloader ca. 5 seconds after starting the box
("ftp 192.168.178.1", password adam2/adam2) and run this command:

	quote SETENV kernel_args ffimage=0

And restart the box.

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

- ffservice start _servicename_
- ffservice stop _servicename_
- ffservice restart _servicename_
	- Self explaining
- ffservice starton _servicename_
	- Start service only if it is enabled
- ffservice enable _servicename_
- ffservice disable _servicename_
	- Enable/disable a service at startup
- ffservice edit _servicename_
	- Edit the service script in $NVRAM/etc/init.d.
	  If it was a symlink, the link will be "broken".
	  Can be restored via purge.
- ffservice config _servicename_
	- Edit the service configuration file in $NVRAM/etc/conf.d,
	  if supported by the service.
- ffservice purge
	- Restore symlink to original service script, if it existed.
	- Removes service configuration file, defaults will be used.
- ffservice list
	- List services
- ffservice status
	- Show status of all services
- ffservice running
	- Exit code is 0/1/2 depending on whether the service is running,
  	  not running or the status is unknown respectively.

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

~~~
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
-H [SPEC=NUM],* : Call setrlimit for given RLIMIT_SPEC before executing
                  client. e.g. STACK=2m,DATA=100m will set hard limit
                  of RLIMIT_STACK/DATA to 2MB/100MB, as well as the 
                  soft limit if its greater than the hard limit.
                  See setrlimit(2)
~~~

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

There are three flavours for the constructed filesystem:

1. Keeping most of the filesystem read-only
2. Using a writeable overlay for the whole filesystem
3. Using a writable copy of the filesystem

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

-	Edit service configuration ("ffservice config buildroot")
-	Add the line BR_USER_OVERLAY=/var/media/ftp/my-root
-	This option will use /var/media/ftp/my-root to store
	new or changed contents of the buildroot filesystem
	in a copy-on-write manner.
-	Preferably, the storage should be located on a mounted
	USB device.
-	NOTE: Be aware that storing files below /var/media/ftp
	exposes them via the NAS service of the box, if enabled!!

Option (3):

-	Edit service configuration ("ffservice config buildroot")
-	Add the line BR_USER_COPY=/var/media/ftp/my-root
-	If the directory does not exist it will be created and 
    	populated with a copy of the buildroot template
	(usr/local/buildroot).
-   	From this time on it will be used as (writable) root filesystem
    	for the buildroot service.

Option 1 is a lightweight mechanism to just start some tools or services that
do not require write access to /usr or other locations which are read-only.

Option 2 is useful when more sophisticated tools shall be used, e.g. when
you want to use python or node.js and add your own modules. But using the
fuse/unionfs makes it quite slow.

Option 3 is a fast alternative for (2), but future updates to the builtroot
template that come with application package updates will not be synchronized
automatically.

For options 2/3 it might be a good idea to remount storages to somewhere else
then the FTP directory.
See 'User defined startup scripts' in [README.md](README.md).

Furthermore, to gracefully handle storage dis- and reconnects the volmgt
service should be used (see below).

All binaries and libraries from the buildroot template directory (usr/bin,
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
- Refer to [AUDIO.md](AUDIO.md) for details.

Music Player Daemon (mpd)
-------------------------
- Uses user space audio tool (via libusb/libmaru) to access an USB audio DAC
- Additional services are
	- upmpdcli (UPNP/DLNA renderer) 
	- ympd (http frontend at port 82) 
- Refer to [AUDIO.md](AUDIO.md) for details

ShairPort-sync Daemon (shairport)
---------------------------------
- Acts as AirPort receiver
- Refer to [AUDIO.md](AUDIO.md) for details

Bluetooth a2dp sink (bluetooth)
------------------------------------
- Reports itself as "FritzBox"
- Tested with Logitech BT stick (CSR chipset)
- Refer to [AUDIO.md](AUDIO.md) for details.

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

See [OPENVPN.md](OPENVPN.md).

Wireguard (wireguard)
---------------------

Experimental. I have observed that the box sometimes hangs when 
removing a wg interface or removing the kernel module.

The wireguard service 

- installs the wireguard.ko kernel module
- create a wireguard interface using the configuration file
  /nvram/ffnvram/etc/wireguard/wg0.conf
- installs an IPv4 UDP forwarding rule for port 51820

tvheadend (tvheadend)
---------------------
Basically, the Fritzbox 6591 is powerful enough to run Tvheadend, but i
consider it still as experimental.
It is only available on FritzOS Version 7.19 or later, and requires the 
buildroot-2020.02 toolchain image.

When enabled, Tvheadend runs as user ffritz, the configuration is stored
in /var/media/ftp/ffritz/.hts. If you want to enable the recorder you need to
explicitly set the recorder path in the tvh configuration menu (i recommend using
an external USB drive).

Caveats:
- I recommend only using one HD tuner at this time until the SatIP server is
  optimized (the libdvbif patch as it was used for the 6490 is not yet ported).
  I have seen cableinfo at 35% CPU load for a single HD stream.

- Don't try to record to some kind of fuse mounted drive (nfs, ovelay, ..).
  It is too slow and memory buffers will eventually lead to the daemon exceeding
  its memory limit (-> crash).

pihole (pihole)
---------------
Requires a specific buildroot setup. See [README-pihole.md](README-pihole.md).

Volume manager daemon (volmgt)
------------------------------

This is a periodic services calling the ffvolume script:

The script manages "ffritz volumes" on internal or (more likely external) filesystems.
The motivation was to become independent from directory names assigned by FritzOS
for mounted drives without having to hook deeply into udev rules.  
Especially when they are surprise extracted the mount point might changes, which in turn
might require services to get reconfigured/restarted.

aFor this purpose, this tool manages directories below the "ffstorage" directory
in the top level of a filesystem.  
Directories below ffstorage are handled as volumes whose name must be unique
over all used partitions (multiple occurences are completely ignored). These directories
are redir mounted to /tmp/storage/_name_, a path which does not change regardless on
where the volume directory is found.

The script is called periodically by the volmgt service (use `ffservice config volmgt` 
to configure).

When executed, the script will handle 
- So far unknown volumes by mounting them as described.
- Removed volumes by  
       - Stopping services known to use them (if enabled)  
       - Attempting to remove the redir mount
- Changed volume paths by  
       - Stopping running services known to use them (if enabled)  
       - Attempting to rebind the volume directory to the new path  
       - Restarting all stopped services  

Services bound to a specific volume can be specified within the file
".ffsvc" in the the volume directory. Just add a newline separated
list of service # names as known by ffservice.
The order is important! Services are started in the order given in the file,
and stopped in the reverse order. 
For the remount operation, only services known to be running (ffservice running
returning 0) will be affected.

Options
-v verbose
-s Show mount status
-f Restart services for remount
-n Don't execute mount operations, just print commands
-X Used internally to not call flock. By default the script makes itself 
   "reentrant" by calling itself with flock (lock file is /var/lock/ffvolume.lck)
   and -X set.

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
