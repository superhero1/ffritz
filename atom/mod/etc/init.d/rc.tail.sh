##########################################################################################
## Betriebsstundenzaehler & Watchdog
##########################################################################################
/bin/run_clock -c /dev/tffs -d
echo init-done >/dev/watchdog
#########################################################################
## Damit auch Oops zum Reboot fuehren
#########################################################################
echo 1 > /proc/sys/kernel/panic_on_oops
#########################################################################
## Damit auch OOM zum Panic/Reboot fuehrt
#########################################################################
echo 2 > /proc/sys/vm/panic_on_oom
## spaeter wenn die tests vorbei sind
## rm -f /var/env
#########################################################################
## set printk level to KERN_ERR
#########################################################################
echo "4" > /proc/sysrq-trigger
if test -x /usr/bin/ethnator ; then
/usr/bin/ethnator -d /etc/init.d/linkdown.sh -u etc/init.d/linkup.sh
fi
#########################################################################
## cleanup - if running, stop debug (0 normal, 1 flush buffer)
#########################################################################
if `ps | grep -v grep | grep -q "cat /dev/debug"` ; then
echo Info: have to stop 'cat /dev/debug'.
echo AVMDBG_EOF 1 >/dev/debug
fi;
#########################################################################
## PTEST: warten, bis der laufenende WLAN-Lifetest beendet ist
#########################################################################
if [ -n "$PTEST_WAIT_PID" ] ; then
wait $PTEST_WAIT_PID
fi
#########################################################################
## modulemem: mit 'fork' <set_m_sleep> Minuten warten, bis alle module gestartet sind.
#########################################################################
if [ -x "/bin/set_modulemem" ] ; then
set_m_sleep=$((10*60))
nohup sh -c "echo \"\$0[\$\$]: ++++fork set_modulemen, sleep ${set_m_sleep}++++\" > /dev/console ; sleep ${set_m_sleep}; echo \"\$0[\$\$]: ++++do set_modulemen++++\" > /dev/console; /bin/set_modulemem;" &
fi
#########################################################################
## MOD: Start telnetd
#########################################################################
/usr/sbin/telnetd -l /sbin/ar7login

exit 0
