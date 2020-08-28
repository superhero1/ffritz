#!/bin/sh

if [ ! -r /tmp/ffbuildroot.conf ]; then
	echo buildroot service not running
	exit 1
fi

. /tmp/ffbuildroot.conf

if [ -z $BR_USER_COPY ]; then
	echo buildroot service must be running in BR_USER_COPY mode
	exit 1
fi

if [ -d $BR_USER_COPY/pihole -a "$1" != "-f" ]; then
	echo pihole seems already installed, try -f to force
	exit 1
fi

test -r /usr/local/share/pihole  || { echo pihole not installed; exit 1; }
test -r $BR_USER_COPY/dev/shm || { echo /dev/shm does not exist; exit 1; }
test -r /usr/local/lib/libmultid.so || { echo /usr/lib/libmultid.so not installed; exit 1; }

lanip=`/sbin/ifconfig lan|grep 'inet addr:'|sed -e 's/.*inet addr:\([0-9.]*\).*/\1/'`
lanip6g=`/sbin/ifconfig lan|grep 'inet6 addr:'|grep Global|sed -e 's/.*inet6 addr: *\(.*\)\/.*/\1/'`
lanip6l=`/sbin/ifconfig lan|grep 'inet6 addr:'|grep Local|sed -e 's/.*inet6 addr: *\(.*\)\/.*/\1/'`

test -z $lanip && { echo failed to determine lan IP; exit 1; }

if [ -z $lanip6g ]; then
	if [ -z $lanip6l ]; then
		echo No local/global ipv6 address
		lanip6=::1
	else
		lanip6=$lanip6l
	fi
else
	lanip6=$lanip6g
fi


echo +++ Extracting template to $BR_USER_COPY
cd $BR_USER_COPY || exit 1
tar xf /usr/local/share/pihole/pihole-template.tar.gz || exit 1
cp /usr/local/share/pihole/pihole $BR_USER_COPY/usr/bin
cp /usr/local/bin/sha256sum $BR_USER_COPY/usr/bin
cp /usr/local/bin/timeout $BR_USER_COPY/usr/bin

# Apply box IP configuration on local network 
#
sed -i -e "s@IPV4_ADDRESS=.*@IPV4_ADDRESS=$lanip/24@" $BR_USER_COPY/etc/pihole/setupVars.conf
sed -i -e "s@IPV6_ADDRESS=.*@IPV6_ADDRESS=$lanip6@" $BR_USER_COPY/etc/pihole/setupVars.conf

# Create some required directories/permissions
#
for d in var/run/pihole/ tmp var/log/lighttpd/ var/run/lighttpd/ var/cache/lighttpd/uploads; do
	mkdir -p $BR_USER_COPY/$d
	chmod 777 $BR_USER_COPY/$d
done

# scripts want stuff in /usr/local
#
if [ -d $BR_USER_COPY/usr/local/bin ]; then
	ln -s /usr/local/pihole $BR_USER_COPY/usr/local/bin
else
	mkdir -p $BR_USER_COPY/usr/local
	ln -sf /usr/bin $BR_USER_COPY/usr/local
fi

# make sure that lighthttpd can run sudo without password
#
grep sudo: $BR_USER_COPY/etc/group | grep www-data >/dev/null || {
	sed -i -e 's/\(sudo:.*\)/\1,www-data/' $BR_USER_COPY/etc/group
}
grep pihole: $BR_USER_COPY/etc/group || {
	echo 'pihole:x:999:www-data' >> $BR_USER_COPY/etc/group
}
grep pihole: $BR_USER_COPY/etc/passwd || {
	echo 'pihole:x:999:999::/pihole:/bin/false' >> $BR_USER_COPY/etc/passwd
}

tmp=/tmp/$$.s
grep -v '^%sudo' $BR_USER_COPY/etc/sudoers > $tmp
echo '%sudo   ALL=(ALL) NOPASSWD: ALL' >> $tmp
mv $tmp $BR_USER_COPY/etc/sudoers

chmod +s $BR_USER_COPY/usr/bin/sudo

echo Updating database
br /opt/pihole/gravity.sh

br pihole -a -p

echo Ready to start pihole service

