Introduction 
============
This image provides some additional software packages for the atom core
which i have implemented, mainly for the purpose of operating the
FritzBox as media player for my amplifier.

Refer to README.md for information how to install it.

Features
========

Music Player Daemon
-------------------
- Uses user space audio tool (via libusb/libmaru) to access an USB audio DAC
- Refer to MPD.md for details
- Startup can be inhibited by creating /var/media/ftp/.skip_mpd

ShairPort Daemon
----------------
- Acts as AirPort receiver
- Refer to MPD.txt for details
- Startup can be inhibited by creating /var/media/ftp/.skip_shairport

Bluetooth a2dp sink
-------------------
- Reports itself as "FritzBox"
- Tested with Logitech BT stick (CSR chipset)
- Output has precedence over mpd and shairport
- Startup can be inhibited by creating /var/media/ftp/.skip_bluetooth

nfs mounter
-----------
The file /var/media/ftp/ffritz/.mtab exists can be created to mount specific
nfs directories to an (existing) location below /var/media/ftp.

The format of the file is an nfs URL, options are those accepted by the
fuse-nfs tool:

    MOUNT  nfs://server/service mount-options

For example, to mount the music database from an external NAS:

    MOUNT Musik/NAS nfs://nas/Multimedia/Music --allow_other

lirc
----
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

OpenVPN
-------

See OPENVPN.md

Miscellaneous tools
-------------------
- ldd
- su
- strace
- tcpdump
- tcpreplay
- mpc
- curl
- rsync
- socat

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
