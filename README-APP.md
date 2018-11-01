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

- lircd execution can be prevented by creating /var/media/ftp/.skip_lircd

OpenVPN
-------

See OPENVPN.md

DVB-C Transport Stream Forwarding
---------------------------------

THIS IS EXPERIMENTAL FOR FRITZOS 7. In the current state it can crash the box!


To enhance DVB-C streaming performance the DVB-C transport stream can be 
forwarded to an external service which generates the RTP packets.

For this the cableinfo daemon is executed with a wrapper library. For details
refer to packages/x86/libdvbfi/README.txt.

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

Packet counters
---------------

The pcount tool (on both arm/atom) can watch various packet counters and
calculate rates.

Usage:
~~~~
 --pp-counters|-p [<all>[,<filter>]]               (ARM only!)
 --l2sw-counters|-l <p>[,<all>[,<filter>]]         (ARM only!)
 --netif-counters|-i <p>[,<all>[,<filter>]]
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

"-p1,MPDSP_frwrd_to_host" "-x2"

This will create a sensor that monitors packets forwarded to ARM due to LUT miss,
which is an indication for a DOS attack ;-).

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
