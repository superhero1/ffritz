Change history of ffritz core integration:
==========================================

NEXT
----
- Support for puma6 and 7 boxes in one branch
	- puma6 (6x90) tested up to 7.12

release 29
----------
- Fix: user patches (like user-oem.patch) were not applied for puma7/arm and puma6 altogether.

----------
- [7.19] Add ffritz.target after multi-user.target to prevent services from starting too early
- Add /nvram/ffnvram/etc/hotplug/udev-mount-sd hook to control where USB storage devices
  are mounted to. This is meant to replace the "remount" service.

release 26
----------
- Removed oem.patch
- Add method for user to add own patches by simply copying them
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
