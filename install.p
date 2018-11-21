--- var_o/install	2018-06-05 20:12:33.000000000 +0200
+++ var/install	2018-06-08 23:04:10.618130000 +0200
@@ -237,9 +237,10 @@
 #! /bin/sh
 if [ $korrekt_version = 0 ] ; then
     echo "error: installype not korrket"
-    echo "set INFO led to off (modul=7, state=1)"
-    /bin/update_led_off
-    exit $INSTALL_WRONG_HARDWARE # warum auch immer: für diese Gerät wird die FW abgelehnt
+## XXX fesc allow installing 6490/6590 on 6590/6490 ...
+#    echo "set INFO led to off (modul=7, state=1)"
+#    /bin/update_led_off
+#    exit $INSTALL_WRONG_HARDWARE # warum auch immer: für diese Gerät wird die FW abgelehnt
 fi
 ##################################################################################
 # Rücksprung nur für die 11.01.xx verhindern
