pi-hole service (experimental)
==============================

This is a snapshot of a pi-hole 5.0 installation. It requires the 
buildroot-2020.02 toolchain and therefore at least FritzOS 7.19.

NOTE: It is recommended to run this with ffritz / FritzOS image
      version 28 (see caveats below)!

Installation (one-time)
-----------------------

- Configure buildroot so that it uses a copied installation:

	ffservice config buildroot

- Set BR_USER_COPY so that it points to a storage location where 
  the buildroot environment is copied to. For example somewhere below
  /var/media/ftp or, preferably, on a USB HDD (see also notes on
  volmgt in [README-APP.md](README-APP.md)).

- Restart buildroot service, this will create the new root filesystem:

	ffservice restart buildroot

- Install pihole into the br environment:

	pihole-setup.sh

  This will install pihole into the buildroot environment, update the initial
  database and prompt for a admin login password.

- If ffritz image version 28 is installed, configure it for pihole use
  (see caveats below), if this is not already done:

	touch /nvram/ffnvram/use_pihole
	/sbin/reboot

Startup
-------

If you want to run pihole as DHCP server as well, run

	ffservice config pihole

and enable the DHCP option.  

To start pi-hole:

	ffservice start pihole

pihole-FTL (dnsmaq variant of pi-hole) should now run, and the GUI should be
available at http://192.168.178.1:85 (or http://fritz.box:85).

To enable pihole at system startup

	ffservice enable pihole

To change the login password:

	br pihole -a -p

CAVEATS
=======

pihole replaces dns/dhcp services in multid by re-starting multid with DNS/DHCP 
ports redirected to port numbers 50000+port.
However, re-starting multid at runtime will  
- multid's DNS server to no longer work
- Make IPv6 to no longer work !

Point 1 is tolerable if one does not plan to stop pihole at runtime. However, 
pi-hole could be configured to ask the local DNS server for local addresses, or
even use the local DNS server for all requests if multids DNS server 
would still work.

For that reason, the base ffritz/FritzOS patch as of version 28 will
check the file /nvram/ffnvram/use_pihole before starting multid initially.
If it exists:  
- A bunch of socat processes is spawned to forward DNS/DHCP requests
- multid is started in redirection mode.

As this makes DNS responses rather slow this option should only be enabled if
you REALLY want to run pihole in the application image!

The pihole start/stop scripts will detect the configuration and not start/stop
the multid process unless nexessary.

Limitations/TODO
----------------
- Only http
- Port pihole setup script instead of providing a binary blob
