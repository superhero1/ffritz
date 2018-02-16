libdvbfi.so / DVB-C Transport Stream forwarding
===============================================

This library has been written as an attempt to speed up streaming of HD channels from
the FritzBox 6490.

It provides a wrapper to the AVM libdvbif.so library used by the cableinfo daemon.
The purpose is to stop sending RTP packets to the actual stream destination, and
directly send the DVB-C transport stream (TS) to an application which can deal with it.
For the FrtizBox this greatly reduces the CPU load and allows sending more HD streams
at the same time.

The intention is to run a forwarder on the destination, which again converts the TS
to RTP (for example dvblast).

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
	<port> is the RTP port number, or -1 for all ports.

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
  
USE CASE: tvheadend and dvblast
- Configure tvheadend to work with 6490 as docuemnted in:
  https://ole-hellmers.de/2017/10/tvheadend-mit-der-fritzbox-6490/

  Make sure to have a uniqe IP address for each tuner, e.g.
  192.168.0.210, 211, 212, 213

- Make sure the port numbers for the 4 tuners are 4 apart each.
  Example: 9000 9004 9008 9012

- Configure libdvb.rc to redirect all RTP traffic to the virtual interfaces as TS to
  RTP port number + 2:

	RTP_REDIR0 192.168.0.210:-1
	RTP_REDIR0 192.168.0.211:-1
	RTP_REDIR0 192.168.0.212:-1
	RTP_REDIR0 192.168.0.213:-1
	UDP_SIZE 26320

- Run the provided fwd script on the host running tvheadend for all of those ports,
  and the 4 assigned IP addresses

	./fwd 9002 9000 192.168.0.210&
	./fwd 9006 9004 192.168.0.211&
	./fwd 9010 9008 192.168.0.212&
	./fwd 9014 9012 192.168.0.213&

Now tvheadend should be happy getting rtp data from the FB via dvblast.

TODO:
- There is definitely a more elegent solution to directly feed tvheadend with the DVB TS
- FB still stutters when there is more than one service used on one tuner (e.g. 2 HD channels)
