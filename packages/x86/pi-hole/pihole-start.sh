#!/bin/sh
#
# Starts pihole services in buildroot envoronment
#

multid_start()
{
	if [ -r /sbin/ffmultid -a -r /tmp/ffnvram/use_pihole ]; then
		echo +++ Restaring DNS/DHCP forwarding to multid
		cd /tmp
		nohup socat -T3 udp-listen:53,reuseaddr,fork UDP:127.0.0.1:50053 > /dev/null&
		nohup socat -T3 udp6-listen:53,reuseaddr,fork UDP6:[::1]:50053 > /dev/null&
		nohup socat -T3 tcp-listen:53,reuseaddr,fork TCP:127.0.0.1:50053 > /dev/null&
		nohup socat -T3 tcp6-listen:53,reuseaddr,fork TCP6:[::1]:50053 > /dev/null&
		nohup socat -T3 udp-listen:67,reuseaddr,fork UDP:127.0.0.1:50067 > /dev/null&
		nohup socat -T3 udp6-listen:547,reuseaddr,fork UDP6:[::1]:50547 > /dev/null&
	else
		echo '!!! restarting multid (DNS might no longer work, restart required)'
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


if [ "$1" = "-h" ]; then
	echo "usage: $0 [-d]"
	echo "  -d: Disable DHCP server in multid so that pihole dhcp can be used"
	exit 0
fi


if [ ! -r /tmp/ffbuildroot.conf ]; then
	echo buildroot service not running
	exit 1
fi

. /tmp/ffbuildroot.conf

if [ -z $BR_USER_COPY ]; then
	echo buildroot service must be running in BR_USER_COPY mode
	exit 1
fi

test -r $BR_USER_COPY/pihole  || { pihole not installed; exit 1; }

# Multid configuration (which ports to move)
_env="LD_PRELOAD=/tmp/.ffpihole/libmultid.so LMD_CHANGE_DNS=1"

if [ "$1" = "-d" ]; then
	_env="$_env LMD_CHANGE_DHCP=1"
fi

need_multid_restart=0

# Change box dns/dhcp service
#
if [ -r /sbin/ffmultid ]; then
	# OS has ffmultid, good, check if it is configured for pihole use
	if [ ! -r /tmp/ffnvram/use_pihole ]; then
		echo WARNING: Consider restarting the box with pihole support:
		echo touch /tmp/ffnvram/use_pihole
		need_multid_restart=1
	else
		# just kill the corresponding socat processes
		#
		for retry in 1 2 3; do
			pids=`ps|grep socat|grep -e listen:53 -e listen:67 -e listen:547 |awk '{print $1}'`
			test -z $pids && break
			echo killing DNS/DHCP socats: $pids
			kill $pids
			sleep 1
		done
	fi
else
	need_multid_restart=1
	echo '!!! Consider updating to ffritz version >=28'
fi

if [ $need_multid_restart -eq 1 ]; then
	echo '!!! killing multid'
	kill_daemon multid 10
fi

sleep 1

echo +++ starting pihole-FTL
br pihole-FTL 

sleep 2

if [ -z `pidof pihole-FTL` ]; then
	echo -- --- failed
	multid_start
	exit 1
fi

br chown www-data /var/log/pihole.log

if [ $need_multid_restart -eq 1 ]; then
	echo '!!! Restarting multid with redirected ports (warning: IPv6 might no longer work, see README)'
	mkdir -p /tmp/.ffpihole
	cp /usr/local/lib/libmultid.so /tmp/.ffpihole/libmultid.so
	
	eval $_env /sbin/multid
fi

echo +++ starting lighttpd
ffdaemon -o /tmp/br /usr/sbin/lighttpd -D -f /etc/lighttpd/lighttpd.conf 

echo +++ done
