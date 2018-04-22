libdvbfi.so / DVB-C Transport Stream forwarding
===============================================

This library has been written as an attempt to speed up streaming of HD channels from
the FritzBox 6490.

It provides a wrapper to the AVM libdvbif.so library used by the cableinfo daemon.
There are three ways to increase speed, from fastest to slowest:

1. Instead of sending RTP data, directly send the TS as UDP fragment
2. Use a rudimentary but faster method to generate RTP packets
3. Use original "cableinfo code" (but multithreaded, which is still faster).

Methods 1 and 2 support large UDP frames (up to 26320 bytes). It depends on the
receiver whether this works. For example

- dvblast supports large UDP TS fragments
- VLC supports large RTP fragments
- tvheadend only supports the default RTP frame size (1516)
	
The purpose of method 1 is to stop sending RTP packets to the actual stream destination,
and directly send the DVB-C transport stream (TS) to an application which can deal with
it.
The intention is to run a forwarder on the destination, which again converts the TS
to RTP (for example dvblast) and sends it to the original destination/port.

USAGE
/usr/local/etc/run_cableinfo [libdvbfi.so path] [arguments to cableinfo]

(re-starts running cableinfo daemon using libdvbfi wrapper with configured parameters
(see below).

CONFIGURATION
The run_cableinfo script is configured by /var/media/ftp/ffritz/libdvb.rc:

RTP_REDIR<index>=<ip>:<port>[:<redir-ip>:<redir-port>]
	A list (index=0..19) of rules which define how RTP traffic shall
	be redirected as transport stream.

	<ip> and <port> are used as match rule for the requester/destination of
	an RTP stream:
	<ip> is an IPv4 address, or 0:0:0:0 as a wildcard for all addresses (TS is sent
	to RTP destination host).
	<port> is
		- the RTP destination port number, 
		- or -1 for all ports.
		- or -2 to activate the experimental mode to encode the stream into RTP 
		  packets to the requesting host/port.

	UDP/TS mode:
	If a stream destination matches ip:port the transport stream is by default sent
	to <ip>:<port>+2. The actual RTP data is no longer sent to <ip>:<port>.

	Optionally, the redirection can be defined in <redir-ip>:<redir-port>:
	<redir-ip> is the ipv4 address to redirect the stream to, or 0.0.0.0 to use the
	same destination as the rtp stream.
	<redir-port> is the destination port to send the transport stream to, or -1 to 
	use the RTP port + 2.

UDP_SIZE
	By default, UDP packets of size 1316 bytes are sent (7 PID frames a 188 bytes).
	If the receiver supports it, this can be increased by factor n
	(up to n=20, which equals 26320 bytes).
  
USE CASE: UDP TS forwarding with tvheadend and dvblast
------------------------------------------------------
- Configure tvheadend to work with 6490 as docuemnted in:
  https://ole-hellmers.de/2017/10/tvheadend-mit-der-fritzbox-6490/

  Make sure to have a uniqe IP address for each tuner, e.g.
  192.168.0.210, 211, 212, 213

- Make sure the port numbers for the 4 tuners are 4 apart each.
  Example: 9000 9004 9008 9012

- Configure libdvb.rc to redirect all RTP traffic to the virtual interfaces as TS to
  RTP port number + 2:

	RTP_REDIR0 192.168.0.210:-1
	RTP_REDIR1 192.168.0.211:-1
	RTP_REDIR2 192.168.0.212:-1
	RTP_REDIR3 192.168.0.213:-1
	UDP_SIZE 26320

- Run the provided fwd script on the host running tvheadend for all of those ports,
  and the 4 assigned IP addresses

	./fwd 9002 9000 192.168.0.210&
	./fwd 9006 9004 192.168.0.211&
	./fwd 9010 9008 192.168.0.212&
	./fwd 9014 9012 192.168.0.213&

Now tvheadend should be happy getting rtp data from the FB via dvblast.

USE CASE: Faster RTP encoding for all requesting hosts
------------------------------------------------------------

	RTP_REDIR0 0.0.0.0

optionally:

	UDP_SIZE 26320

if the receiver (e.g. VLC) supports it.

USE CASE: Just run the patched cablieinfo to make it multi-threaded
-------------------------------------------------------------------

	RTP_REDIR0 1.1.1.1

This is a dummy entry to have the modified cableinfo started at all.


TODO:
- The RTP packet generation is quite rudimentary. I can't guarantee it will work with
  all clients (tested VLC and tvheadend).
