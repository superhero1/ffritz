pi-hole service (experimental)
==============================

This is a snapshot of a pi-hole 5.0 installation. It requires the 
buildrtoot-2020.02 toolchain and therefore at least FritzOS 7.19.

Installation (one-time)
-----------------------

- Configure buildroot so that it uses a copied installation:

	ffservice config buildroot

- Set BR_USER_COPY so that it points to a storage location where 
  the buildroot environment is copied to. For example somewhere below
  /var/media/ftp or, preferably, on a USB HDD.

- Restart buildroot:

	ffservice restart buildroot

- Install pihole into the br environment:

	pihole-setup.sh

  This will install pihole into the buildroot environment, update the initial
  database and prompt for a admin login password.

Startup
-------

Run

	pihole-start.sh

This will

- kill multid
- start pihole-FTL (pihole/dnsmasq daemon)
- restart multid without DNS server
- start lighttpd at port 85 (which is the pihole web GUI)

If you want to run pihole as DHCP server as well, run

	ffservice config buildroot

and enable the DHCP option.  
This will disable the box dhcp server in multid.

To change the login password:

	br pihole -a -p


Limitations/TODO
----------------

- Only http
- Port pihole setup script instead of providing a binary blob
- more testing ..


