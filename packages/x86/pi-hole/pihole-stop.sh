#!/bin/sh
#
# Stops pihole services in buildroot envoronment
#
multid_start()
{
	if [ -r /sbin/ffmultid -a -r /tmp/ffnvram/use_pihole ]; then
		cd /tmp
		nohup socat -T3 udp-listen:53,reuseaddr,fork UDP:127.0.0.1:50053 >/dev/null&
		nohup socat -T3 udp6-listen:53,reuseaddr,fork UDP6:[::1]:50053 >/dev/null&
		nohup socat -T3 tcp-listen:53,reuseaddr,fork TCP:127.0.0.1:50053 >/dev/null&
		nohup socat -T3 tcp6-listen:53,reuseaddr,fork TCP6:[::1]:50053 >/dev/null&
		nohup socat -T3 udp-listen:67,reuseaddr,fork UDP:127.0.0.1:50067 >/dev/null&
		nohup socat -T3 udp6-listen:547,reuseaddr,fork UDP6:[::1]:50547 >/dev/null&
	else
		/sbin/multid
	fi
}


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
if [ -r /sbin/ffmultid -a -r /nvram/ffnvram/use_pihole ]; then
	echo +++ restarting redirection to multid DNS/DHCP ports
	cd /tmp
	nohup socat -T3 udp-listen:53,reuseaddr,fork UDP:127.0.0.1:50053 >/dev/null&
	nohup socat -T3 udp6-listen:53,reuseaddr,fork UDP6:[::1]:50053 >/dev/null&
	nohup socat -T3 tcp-listen:53,reuseaddr,fork TCP:127.0.0.1:50053 >/dev/null&
	nohup socat -T3 tcp6-listen:53,reuseaddr,fork TCP6:[::1]:50053 >/dev/null&
	nohup socat -T3 udp-listen:67,reuseaddr,fork UDP:127.0.0.1:50067 >/dev/null&
	nohup socat -T3 udp6-listen:547,reuseaddr,fork UDP6:[::1]:50547 > /dev/null&
else
	echo '!!! restarting multid (DNS might no longer work, restart required)'
	kill_daemon multid 10
	/sbin/multid
fi
