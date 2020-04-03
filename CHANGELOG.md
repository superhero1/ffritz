Change history of application package
=====================================

Next
----
- Buildroot version 2020-02 (only for FritzOS 7.19 onwards)

Release puma7-14
----------------
- Don't activate any service except buildroot on a fresh installation
- Fix LD_LIBRARY_PATH for ff wrapper (required for 7.19)
- ffimage=0 option in kernel_args EFI parameter will prevent application 
  image startup.

ISSUES
- wireguard
	- Kernel panic in 7.13 on "ip link up", will not load here
	- Same on 7.19, so kernel module will not be built

Release puma7-13
----------------
- linux kernel modules
	- no longer required (cdc-acm part of FritzOS)
- Remove athtool
- use AVM kernel/defconfig (from rootfs) for building modules
- Add wireguard service, module, wg-quick and wg tool
- Replace/fix shairport with shairport-sync
- Added nfs server (user space unfsd).
- Buildroot: The /var filesystem is now a unionfs with ramdisk instead of nvram.


Release 12
----------
- Add feature to buildroot to use FS overlay on flash
	- Configuration file /tmp/ffnvram/ffbuildroot.conf
	- Parameter BR_USER_OVERLAY
		Specifies directory to be used as writable
		overlay for the whole root filesystem /tmp/br
- Buildroot service script kills all chrooted processes before
  unmounting.
- Bump buildroot version to buildroot-2019.05-rc2
- Fix syntax error buildroot userconfig target
- Added/changed some default buildroot settings:
	- Python options and default packages:
		PYTHON3_PY_PYC
		PYTHON3_CURSES
		PYTHON3_SSL
		PYTHON_SETUPTOOLS
		PYTHON_PIP

	- Added
		DOSFSTOOLS
		DOSFSTOOLS_FATLABEL
		DOSFSTOOLS_FSCK_FAT
		DOSFSTOOLS_MKFS_FAT
		E2FSPROGS
		E2FSPROGS_RESIZE2FS
		EXFAT
		EXFAT_UTILS
		F2FS_TOOLS
		SSHFS
		BWM_NG
		ETHTOOL
		FPING
		NMAP_NMAP
		SUDO
		HTOP

- TODO
	Add (buildroot service) option to relocate a USB storage as
	overlay so that it is not exposed via /var/media/ftp.

Release 11
----------
- Fix kernel header version for toolchain
  (caused some issues in chroot environment)

Release 10
----------
- Use latest buildroot (2016.05 -> snapshot 20190428)
	- Tons of new versions

- Application package
	- Added ympd http frontend for mpd (port 82)
	- Added list/restart options to ffservice
	- Minor stuff for new buildroot
	- Simplified OpenVPN setup. Forwarding rule is now applied
	  automatically (pcplisten) without having to define one in the 
	  FritzOS GUI.

	- INCOMPATIBILITIES
		- OpenVPN : the OpenVPN.conf file needs to be changed:
		  In /tmp/ffnvram/root-ssh/openvpn, change "local" from
		  
		  192.168.178.253   (or whatever was configured)

		  to the main address of the box (e.g. 192.168.178.1)

- TODO
	- ARM tools with new buildroot

Release 9
---------
- Application package
	- Added basic service mechanism (See README-APP.md)
	- Added "buildroot" service to have buildroot available as chroot
	  environment.
		- in /usr/local/buildroot
		- init script will make chroot-able fs in /tmp/br
		- ramdisc overlay with unionfs-fuse
		- experimental, far goal is to use buildroot for all
		  services
	- mpd: Enhanced Recorder plugin
		- "parent" property will create a recorder instance
		  which just tells named instance to copy current
		  title to archive_path
		- archive_path: like format_path, will copy current
		  title there when told by a client instance to do so.
		- delete_after_record: Delete current title after
		  recording it.
	  The purpose is to be able to capture the complete title that
	  is currently playing by:
	  - Continuously running an instance with "delete_after_record"
	  - only save the title when toggling the run state of an
	    instance with a "parent" attribute (stop->start).
	- DVB-C: Some adaptions to wrapper library, looks now stable.
	- Change startup/service scripts
		- now in /tmp/ffnvram/etc/inid.d, linked to etc/rc.d
		  for execution
		- can be changed and saved with nvstore

 - TODO
 	- libdvbfi: sometimes stops working after closing stream
		- FOS 7.02: MIGHT CRASH THE BOX!
		- FOS 7.08 Labor: Seems stable

Release 8 (FritzOS7 only)
-------------------------
- Application package version 8
	- Added support for PRTG Network Monitor (see README-APP.md)
	- Provide all busybox apps as links in /usr/local/bin
	- Some enhancements to ffdaemon for service management
	- Added netsnmp, dnsmasq  (for manual start)
	- Added unionfs-fuse
	- Added athtool (ext. switch config)
	- Updated btstack repository

Release 7b(eta)
---------------
- mpd 0.20.5 -> 0.20.18
- mpdclient 2.9 -> 2.11
- Bluetooth btstack Apr 10 2018 [0a42fd2391f2ec0205a46de679e23220490c6da8]
- libupnp 1.15 -> 1.16
- upmpdcli 1.12 -> 1.16
- Option for DVB-C to speed up rtp packet transimission. Make it multi-threaded.

Release 6
---------
- Added gdb, gdbserver, ltrace
- Added DVB-C transport stream forwarding to offload Atom CPU.
  See README-dvb.txt

Release 5
---------
- lirc: Eliminate endless loop when disconnecting irtoy at runtime
- usbplayd: Finally support different discrete sample rates of endpoints (somehow)
	- sndlist lists all supported rates
	- adding "-r 44100" to usbplayd.conf avoinds resampling (if endpoint supports it)
	- Some (my) endpoint report an error if the rate request command is sent, but do it
	  anyhow correctly, so error is ignored.
- bluetooth:
	- store session/pairing data persistently in /var/media/ftp/ffritz/bt
- Remove 100s startup delay unless box is booting

Release 4
---------
- Fix tmpfs memory leak: fflogrotate got lost along the way,
  /var/log/ffritz.log could become big over time.
- Fix upmpdcli startup issue
- Limit logrotate backup to 250K
- ffdaemon: can be stopped with SIGTERM, will reap worker
- Added etc/ffshutdown
- Added -u option to etc/usrmount

Release 3
---------
- Add 100 second startup angst delay.
  ffritz services (USB, OpenVPN) get in conflict with fb startup,
  especially on 6.85.

Release 2
---------
- Fix irexec (failed to execute commands due to wrong sh path)
- Updated libusb version to 1.0.21
- Started Bluetooth support
	- Requires updating /var/media/ftp/ffritz/usbplayd.conf
	  with

    USBPLAYD_ARGS=-P /var/tmp/mpd.fifo:44100 -P /var/tmp/shairport.fifo:44100 -P /var/tmp/bt.fifo:44100

Release 1
---------
- Renamed from release 14 of combined core/application release
- Re-packaged as squashfs binary to be installed using ffinstall

TODO
----
- Bluetooth
	- Play pipe should not be opened until playback
	  starts.
	- Provide proper pairing (e.g. only when pressing
	  WPS button..)


Change history of ffritz core integration:
==========================================

next
----
- Add method for use to add own patches by simply copying them
  as user-*.patch into the atom/arm subfolder.

release 25
----------
- Add support for ARM toolchain/application image.

release 24
----------
- Add support for Lab Firmware 7.19
	- New init system
	- kernel 4.9.175
- Add Makefile option to enable console
- Move configurable make settings to conf.mk
- uimg tool: Partition files names can be without content identifier.
- ffimage=0 option in kernel_args EFI parameter will prevent mounting of 
  image startup.

TODO

release 23
----------
- Toolchain
	- buildroot
		- Use Release 2019.05
		- change to glibc from uClibc.
			- Use same as AVM (2.23) to avoid two different
			  libc's (LD_LIBRARY_PATH does not work for libc).
	- uimg: Fix help message
- privatekeypassword
	- remove check for uClibc
- Adapt rc scripts
	- dont use md5 hash any more
- Adapt switch_bootbank script
- Change atom squashfs options: set xz compression, bfs export, no xattrs
- Statically linked busybox/telnetd for now
- application package must contain glibc-version file to distinct from uclibc image
- Force firmware_version to avm
- Force CONFIG_RTL=y
- Move "nvram" to /nvram filesystem.
	The nvram used to be an encrypted tar file extracted to /tmp/ffnvram.
	It is now located in /nvram/ffnvram, which is a ext4 filessytem on
	the eMMC (ca. 5MB free). The startup script migrates the existing
	data to /nvram and uses it from there on.
        nvsync is no longer necessary but can still be called.


release 22
----------
- Atom: Fix initial telnetd and ssh login with password authentication.
- ARM: New binaries for buildroot snapshot 20190428
- ARM: added more tools to base image:
	tcpdump etc, gdb, gdbserver, ltrace, strace

release 21
----------

!! broken, no telnetd, no ssh password auth

- Use latest buildroot (2016.05 -> snapshot 20190428)
  Tons of new versions ..
  	- openssl changed to 1.1.0
	  default message digest is changed here from md5 to sha256. To keep
	  nvram accessible between version changes i still use md5 for now,
	  but this might eventually change. 
	  This image supports both for decrypting nvram.

release 20
----------
- Fix startup issue on FRITZOS 7.02 when previously no ffritz was installed.
- Do not download ARM extension package by default
- Init script /etc/init.d/S97-ffusr executes /tmp/ffnvram/etc/rc.user, if
  it exists.

release 19
----------
- Support for FritzOS 7 (only)
	- New buildroot toolchain -> new uClibc etc. ->
		all binaries incompatible to previous versions
	- patch var/install to allow 6590 image installation on 6490 (WLAN wont work!)
	- Distinct installed application package for uclibc 0.x and 1.x
	- Remove annoying login outputs from /etc/profile, add /sbin/usr/sbin to path
    	- Fix reassignment of console to 1st login
	- ARM: Reintegrated pswtool

release 18
-------------------------------
- Various fixes to build system
- Auto-load source/binary images.
- Update to AVM source version 6.87
- Atom: Always rebuild kernel modules (cdc-acm)
- Add make help
- Add make targets for rebuilding packages and overwriting pre-built binaries.
- Properly build and install be squashfstools from freetz
- dropbear 2016.74 -> 2018.76
- prepare openssh

release 17
-------------------------------
- Fix permission of / directory to 755 to make dropbear happy
- Makefile: integrate arm image from build directory if it exists
- Tested with firmware 6.87

release 16
-------------------------------
- Tested Firmware 6.85
	- Update might require re-enabling the OpenVPN
  	  forwarding rule.

release 15 
-------------------------------
- Add support for Firmware 6.83
- Arm
	- Don't start temporary telnetd any more
	- Don't invoke atom startup
- Atom
	- Application tarball is no longer integrated into root
	  filesystem
		- Replaced by sqaushfs image mounted at startup
		  (/var/media/ftp/ffritz/data/ffimage.bin)
		- Only if sha256 checksum matches
	- Support for 6.83 (older versions no longer tested)
	- Replace nvram access by an encrypted storage on the nas
		(/var/media/ftp/ffritz/data/ffstore.dat)
		- Runtime copy is in /tmp/ffnvram
		- Added nvsync tool to store current state of
		  ffnvram to ffstore
		- Lacking a ffstore.dat, ffnvram is populated
		  traditionally from /nvram
	- Dropbear, openssl and telnetd are part of default
	  squashfs install image.
	- Start temporary telnetd
	- mpd
		- add libnfs support
	- usrmount/.mtab
		- uses fuse-nfs (kernel no longer supports nfs).
		- The options field in .mtab has changed and needs
		  to be adjusted:
		  It is the servers URI in libnfs format
		  (nfs://server/share), followed by additional options
		  for fuse-nfs.
	- added openssl binary
	- add build of privatekeypassword
	- Change license of source to GPLv3 to be compatible with
	  apache code

release 14
----------
- Atom
	- Add OpenVPN (experimental). See OPENVPN.md

- Arm
	- Add counter support to athtool
	- Add pswtool to access internal Puma6 L2 switch

release 13
----------
- Atom
	- buildroot
		- New gcc version (4.7.3 -> 4.9.3)
		- New binutils version (2.21.1 -> 2.23.1)
			- Host gcc 4.7 no longer required
	- New mpd version (0.19.21 -> 0.20.5)
		- mpd/upmpdcli now support LPCM/l16 audio format
- Arm
	- version 0.3 of arm package
		- added gdbserver
		- Added athtool to access box switch

release 12
----------
- Atom
	- Added upmpdcli
		- UPNP/DLNA renderer front-end for mpd
		- l16/LPCM audio is currently not supported
		  (would require latest mpd, which does not compile in this
		  toolchain)
	- Auto-start irexec
	- Moved irexec config file to /var/media/ftp/ffritz/irexec.lircrc

release 11
----------
- Atom
	- lirc
		- switch back to main lirc repository
		- replaced irdroid driver with fixed irtoy
		  NOTE: _This requires replacing/merging existing
		  lirc_options.conf with etc/lirc_options_dfl.conf!_
		- lircd sometimes crashed after the very first start.
		  Workaround is to start it in self-respawning mode
		  (via ffdaemon).
	- usbplayd
		- Fix hanging daemon
	- added ffdaemon script to start service as daemon
	- some startup/daemon output is logged to /var/tmp/ffritz.log,
	  added simple logrotate
	- moved usbplayd.pid to /var/run
	- moved mpd.pid to /var/run/mpd
	- Add udev rule for /dev/ttyACM* permission

release 10
----------
- Atom
	- Toolchain fixes for clean build
	- Added lirc
	- Added socat

release 9
---------
- Atom
	- Add libid3tag / id3 tag support to mpd
	- Fix access rights to /var/tmp/volume file at startup
	- Make usbplayd self-respawning if it crashes
	- Dont log usbplayd to /var/tmp to avoid hogging ramfs space
	- Forward dropbear stderr outputs to /dev/console
	- Create ssh symlink to dbclient
	- Added curl binary
	- Added rsync binary
- ARM
	- Make sure that ssh stuff and passwords in /nvram are not cleared
	  when a factory reset is performed (by means of entries in 
	  /etc/docsis/nvramdontremove). Only for FW 6.6x and later.
	- Do not apply ipv6 patch for 6.63 and later
	- Added Makefile option to integrate separate arm package into root
	  filesystem (ffritz-arm-XX.tar.gz, see below).
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
	- Replaced usbplay with usbplayd, which is fifo based and accepts
	  several inputs. It also performs sample rate conversion if required.
	- Accordingly, mpd uses fifo output driver instead of pipe driver
	- Added shairport (AirPort audio receiver)
	- Added user mount table (var/media/ftp/ffritz/.mtab)
- ARM
	- Added /usr/local/etc/switch_bootbank script to help bank switch
	  without having to invoke the bootloader.
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
---------
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

Standalone Package History
==========================

ffritz-arm-XXX.tar.gz
---------------------
- 0.4
    - Added pswtool (for interlanl L2 switch)
    - Some fixes to athtool (counter rate calculation)
- 0.3
    - Added athtool to access box switch (see description above)
- 0.2
    - Added curl
    - Added rsync
- 0.1
    - Created, to be installed to /var/media/ftp/ffritz-arm
      Contains some libs, tcpreplay, dump, ..., strace, ldd
