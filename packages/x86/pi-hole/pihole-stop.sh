#!/bin/sh
#
# Stops pihole services in buildroot envoronment
#

kill_daemon()
{
	proc=$1
	to=$2
	while [ "x`pidof $proc`" != "x" -a $to -gt 0 ]; do
		killall $proc
		sleep 1
		to=`expr $to - 1`
	done

	test $to -eq 0 && { echo failed to stop $proc; return 1; }
}

if [ ! -r /tmp/ffbuildroot.conf ]; then
	echo buildroot service not running
	exit 1
fi

#. /tmp/ffbuildroot.conf
#
#if [ -z $BR_USER_COPY ]; then
#	echo buildroot service must be running in BR_USER_COPY mode
#	exit 1
#fi

echo +++ stopping lighttpd
ffdaemon -K lighttpd
echo +++ stopping pihole-FTL
kill_daemon pihole-FTL 10
echo +++ restarting multid 
kill_daemon multid 10
/sbin/multid
