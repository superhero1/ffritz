#!/bin/sh
#
# initialize setupVars.conf with box settings
# usage
# pihole-config [-i] [setupVars.conf]
# - initial setup: also take over dhcp configuration from fbox
#

if [ "$1" = "-h" ]; then
	echo "usage: $0 [-i] [setupVars.conf-file]"
	echo "-i Perform IP and DHCP config (only IP otherwise) in setupVars.conf"
	exit 0
fi

i_flag=0
if [ "$1" = "-i" ]; then
	i_flag=1
	shift
fi

if [ ! -z "$1" ]; then
	conf=$1
else
	. /tmp/ffbuildroot.conf

	test -z $BR_USER_COPY && { echo buildroot service must be running in BR_USER_COPY mode >&2; exit 1; }

	conf=$BR_USER_COPY/etc/pihole/setupVars.conf

	test -r $conf || { echo "$conf not present, pihole installed?" >&2; exit 1; }
fi

test -r "$conf" || { echo "$0: no setupVars.conf file found ($conf)" >&2; exit 1; }

lan4ip=`/sbin/ip -4 addr show dev lan scope global | grep -m 1 inet  | awk '{print $2}'`
lan4ip_base=`echo $lan4ip | sed -e 's@/.*@@'`
lan6ip=`/sbin/ip -6 addr show dev lan scope global | grep -m 1 inet6 | awk '{print $2}'`
domain=`aicmd multid  dhcpd context | grep Domainname | sed -e 's/.*:[ ]*//'`
ip_range=`aicmd multid dhcpd interfaces | grep -A1 "SERVER lan:" | tail -1`

sed -i 	-e "s@IPV4_ADDRESS=.*@IPV4_ADDRESS=$lan4ip@" \
	-e "s@IPV6_ADDRESS=.*@IPV6_ADDRESS=$lan6ip@" \
	-e "s@PIHOLE_DOMAIN=.*@PIHOLE_DOMAIN=$domain@" \
	 $conf

grep _ADDRESS $conf
grep PIHOLE_DOMAIN $conf

if [ $i_flag -eq 1 -a "$ip_range" != "" ]; then
	set $ip_range x
	if [ "$1" != "" -a "$2" = "-" -a "$3" != "" ]; then
		sed -i  -e "s@DHCP_START=.*@DHCP_START=$1@" \
		        -e "s@DHCP_END=.*@DHCP_END=$3@" \
			-e "s@DHCP_ROUTER=.*@DHCP_ROUTER=$lan4ip_base@" \
			$conf

			grep DHCP_ $conf
	else
		echo bad IP range config: $ip_range >&2
		exit 1
	fi
fi
