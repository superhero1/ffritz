This tool  has been written as an attempt to speed up streaming of HD channels from
the FritzBox 6490.

This library is a wrapper to the AVM libdvbif.so library, and also wraps around 
some functions in libavmcsock.so.
The purpose is to stop sending rtp packets to the actual stream destination, and
directly send the FVB transport stream to an adjacent port (rtp port + 2).

The intention is to run a forwarder on the destination, which again converts the TS
to RTP (for example dvblast).

USAGE
- Binary patch the original cableinfo binary from the FB firmware so that it loads
  libdvbfi.so instead of libdvbif.so.
  E.g. use the provided "spatch" script:

	spatch cableinfo cableinfo_p libdvbif.so libdvbfi.so

- Stop original cableinfo process
	
	killall cableinfo

- Optional: Explicitly define which RTP ports to block and send TS instead:
  export "RTP_PORTLIST"=9000,9004,9008,9012
- run patched process in the directory where libdvbfi.so resides:

	export LD_LIBRARY_PATH=`pwd`
	LD_PRELOAD=./libdvbfi.so ./cableinfo_p -f 

  (omit -f to run in background)
  
USE CASE: tvheadend (requires dvblast)
- Configure tvheadend to work with 6490 as docuemnted in:
  https://ole-hellmers.de/2017/10/tvheadend-mit-der-fritzbox-6490/

  Make sure to have a uniqe IP address for each tuner, e.g.
  192.168.0.210, 211, 212, 213

- Make sure the port numbers for the 4 tuners are 4 apart each.
  Example: 9000 9004 9008 9012

- Run the fwd script for all of those ports, and the 4 assigned IP addresses

	./fwd 9002 9000 192.168.0.210&
	./fwd 9006 9004 192.168.0.211&
	./fwd 9010 9008 192.168.0.212&
	./fwd 9014 9012 192.168.0.213&

Now tvheadend should be happy getting rtp data from the FB.

Environment parameters: 
RTP_PORTLIST - A list of UDP/RTP ports on the target to block and replace with TS
               data (port + 2). If omitted, all data is blocked this way.
               To allow "default behavior" of cableinfo, set to -1

UDP_SIZE     - Size in bytes of a UDP fragment. The default is 1316.
	       Bigger values add lots of performance, but the other end must be 
	       able to deal with it (dvblast does not)

TODO:
- There is definitely a more elegent solution to directly feed tvheadend with the DVB TS
- FB still stutters when there is more than one service used on one tuner (e.g. 2 HD channels)
	- Eliminate packet sent back to cableinfo
	- Tune forwarder to acceppt UDP packets > 1316
