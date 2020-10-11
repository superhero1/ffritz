Change history of application package
=====================================

Release puma7-19
================
- Add alsa support
  - Build USB sound kernel modules for kernel 4.9.199 (FritzOS >= 7.19)
  - Added config option to usbplayd
    - REQUIRED: run "ffservice reconf usbplayd" and adjust USE_ALSA
  - Add alsa support for shairport-sync
    - Add executable for audio start in shairport-sync.conf:
    - /nvram/ffnvram/etc/shairport-play-start and stop
    - OPTIONAL: change to interact with mpd (start script stops mpd)
  - Add alsa output to default mpd.conf
    - REQUIRED: change in user mpd.conf to support ALSA
- added ffstart/stop tools
- pihole:
  - Run pihole-localhost service to make local names visible to pihole
    (unless pihole DHCP is used)
  - Interact with base image to make use of temporary socat processes
    to redirect DNS/DHCP to multid.
- Some build changes
  - external toolchain support, toolchain blob downloaded from net.
- Add wireguard support for FritzOS 7.21
- Add ffinstall host tool to install app image via ssh

Release puma7-18
----------------
- Added volmgt service/ffvolume script to mount drives to fixed locations.
- Added missing pihole service file
- shutting down will attempt to stop all services, not just the enabled ones
- Added cash tool.
- Bump shairport-sync to 3.3.7rc2.
- ffservice purge will only restore old service script

Release puma7-17
----------------
- Merge pihole branch (see [README-pihole.md](README-pihole.md))

Release puma7-16
----------------
- Add setrlimit options to ffdaemon.
- usrmount: Try to create mount directory before mounting
- tvheadend: apply stack and memory limit via ffdaemon
- ffservice: add edit, config and purge option.
  Adapted default service files.
- buildroot: now using ffservice configuration file.
  Old file (ffbuildroot.conf) is transferred.
- usbplayd: now using ffservice configuration file.
  Old config file (usbplayd.conf) no longer used.
- mpd: service supports options to disable ympd/upmpdcli
- bluetooth: can set AP name in config options.

Release puma7-15
----------------
- Buildroot version 2020-02 (only for FritzOS 7.19 onwards)
	- Add support for different buildroot versions living alongside.
	  (see [conf.mk.tpl](conf.mk.tpl))
	- Add PARALLEL option to conf.mk to perform buildroot parallel build.
- Add non-ELF binaries from buildroot usr/(s)bin to /usr/local/bin.
- tvheadend: Added patch/support for buildroot-2020.02
- shairport-sync: version 3.3.5

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
	- Added support for PRTG Network Monitor (see [README-APP.md](README-APP.md))
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
