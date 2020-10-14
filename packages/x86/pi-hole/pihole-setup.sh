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

INSTDIR=/usr/local/buildroot/usr/share/pihole

test -r $INSTDIR  || { echo pihole not installed; exit 1; }
test -r $BR_USER_COPY/dev/shm || { echo /dev/shm does not exist; exit 1; }
test -r /usr/local/lib/libmultid.so || { echo /usr/lib/libmultid.so not installed; exit 1; }

echo +++ Extracting template to $BR_USER_COPY
cd $BR_USER_COPY || exit 1
tar xf $INSTDIR/pihole-template.tar.gz || exit 1
cp $INSTDIR/pihole $BR_USER_COPY/usr/bin
cp /usr/local/bin/sha256sum $BR_USER_COPY/usr/bin
cp /usr/local/bin/timeout $BR_USER_COPY/usr/bin


# Apply box IP/DHCP configuration on local network 
#
pihole-config -i $BR_USER_COPY/etc/pihole/setupVars.conf

# Create some required directories/permissions
#
for d in var/run/pihole/ tmp var/log/lighttpd/ var/run/lighttpd/ var/cache/lighttpd/uploads; do
	mkdir -p $BR_USER_COPY/$d
	chmod 777 $BR_USER_COPY/$d
done

# scripts want stuff in /usr/local
#
if [ ! -r $BR_USER_COPY/usr/local/bin/pihole ]; then
	if [ -d $BR_USER_COPY/usr/local/bin ]; then
		ln -s /usr/local/pihole $BR_USER_COPY/usr/local/bin/
	else
		mkdir -p $BR_USER_COPY/usr/local
		ln -sf /usr/bin $BR_USER_COPY/usr/local/bin
	fi
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

br chmod 775 /etc/pihole
br chown pihole  /etc/pihole /etc/pihole/*
br chgrp www-data  /etc/pihole /etc/pihole/*

echo Updating database
br /opt/pihole/gravity.sh

# If terminal, flush input and ask for default password
#
test -t && {
while read -t 0.01; do :; done

br pihole -a -p
}

echo Ready to start pihole service

