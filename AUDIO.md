I thouhgt it would be nice to have mpd (music player daemon) and shairport
(AirPlay receiver) running on the Atom core of the FritzBox, with some
USB DAC/Codec as output.

Audio components:

~~~
                        +---------+
                        | USB DAC |
                        +----+----+
                             ^
                             |
               +-------------+----------------+
               | libusb/libmaru/libsamplerate |
               +-------------+----------------+
                             ^
                             |
                     +-------+--------+
           +-------->+    usbplayd    +<-------+-----------------+
           |         +-------+--------+        |                 |
           |                 ^                 |                 |
           |                 |                 |                 |         
           |                 |                 |                 |        
      +----+-----+   +-------+--------+   +----+----+        +---+----+
      | mpd.fifo |   | shairport.fifo |   | bt.fifo |        | volume |
      +----+-----+   +-------+--------+   +----+----+        +---+----+
           ^                 ^                 ^                 ^
           |                 |                 |                 |
        +--+--+        +-----+-----+    +------+---------+    +--+--+
   +--->+ mpd +<--+    | shairport |    | a2bt_sink_demo |  >>> all |
   |    +-----+   |    +-----------+    +----------------+    +-----+
   |              |
   |              |
+--+---+     +----+-----+
| ympd |     | upmpdcli |
+------+     +----------+
~~~

usbplayd
========

This daemon is started to provide audio output. It polls a set of named pipes
and plays data from one of them to the selected usb audio device. If required it
will convert the sample rate.

usbplayd is configured in /var/media/ftp/ffritz/usbplayd.conf.
The default content is:

    USBPLAYD_ARGS=-P /var/tmp/mpd.fifo:44100 -P /var/tmp/shairport.fifo:44100 -P /var/tmp/bt.fifo:44100

Which means that three pipes are generated, one for mpd, one for shairport
and one for bluetooth.
The default sample rate is 44100 (i.e. mpd and shairport need to provide this
sample rate).

If you have a usb audio device which only supports 48000Hz usbplayd will
resample the "fifo rate" to the "device rate" using libsamplerate.

If you have a device that supports configurable rates you might
want to add the parameter "-r 44100" to configure the usb device to this
rate. This will avoid resampling if the default rate differs, but not all
devices seem to support this.

To get a list of usb devices and the attributes, call "usbplayd -l".

The output volume of the device is adjusted in /var/tmp/volume. This file 
is polled by usbplayd and the volume is adjusted according to the ASCII integer
(0..100) in the file.
The modified mpd/shairport/bluetooth binaries in this package supports this feature.

Note that the shairport fifo has precedence over the mpd fifo, i.e. if 
shairport starts playing, mpd is effectively muted until shairport stops.
This behaviour is determined by the order of the -P parameters to usbplayd.

Sample rate conversion
----------------------

usbplayd will use libsamplerate if pipe and device sample rate mismatches.
The maximum supported ratio is 4 (device rate / pipe rate).

libsamplerate has different algorithms:

- 0 : `SRC_SINC_BEST_QUALITY`
- 1 : `SRC_SINC_MEDIUM_QUALITY`
- 2 : `SRC_SINC_FASTEST`
- 3 : `SRC_ZERO_ORDER_HOLD`
- 4 : `SRC_LINEAR`

The "best quality" algorithm results in a CPU load of ca 50% (44.1KHz -> 48KHz).
The default is 2 (`SRC_SINC_FASTEST`) (ca. 7% load), a good compromise is
`MEDIUM_QUALITY` with 17% load (although i don nott hear a difference).
Add the `-c <algo-number>` switch to `USBPLAYD_ARGS` to change the default.

MPD
===

mpd is started via the mpd service. By default it will use the fifo output
plugin to write data to /var/tmp/mpd.fifo at a sample rate of 44100Hz.

The configuration file is /var/media/ftp/ffritz/mpd.conf.
mpd runtime files (database, etc) are stored in var/media/ftp/ffritz/.mpd.

The default Music database is /var/media/ftp/Musik.

If you do not want to start mpd:

	ffservice disable mpd
	ffservice stop mpd

The CLI client for mpd (mpc) is available on the atom core.

Volume Control
--------------
The mpd binary in this package has been modified to support hardware volume
control in combination with the pipe/fifo output plugins by means of a
"volume" file (see usbplayd).

This is done by setting the mixer type of pipe/fifo in mpd.conf to "hardware"
and specifying the "volume_file" attribute.

The default mpd.conf uses /var/tmp/volume as expected by usbplayd.

Recorder plugin
---------------
I was always annoyed that in the middle of listening to a title i come to the
conclusion that i would have liked to record it.
So i enhanced the recorder plugin to be able to continuously record, but 
discard the title after it has finished, except when i want to keep it.

Sample setup:

~~~
audio_output {
        type            "recorder"
        name            "Recorder-One"
        format_path     "[/tmp/samba/[%artist%-]%title%.ogg]"
        archive_path    "[/var/media/ftp/Musik/NAS/rip/archive/[%artist%-]%title%.ogg]"
        encoder         "vorbis"
        quality         "10"
        delete_after_record "1"
}
audio_output {
        type            "recorder"
        name            "Recorder-KeepCurrent"
        parent          "Recorder-One"
}
~~~

The "Recorder-One" instance is meant to be running always. It will store
the current title to /tmp and discard it afterwards ("delete_after_record"
attribute).

The "Recorder-KeepCurrent" instance will do nothing, except telling the
"Recorder-One" instance to move the current title to its "archive_path" before
deleting it.
This is triggered by an "disable->enable" transition of the "Recorder-KeepCurrent" 
instance any time during the runtime of the current title.

After having saved the title, the "Recorder-One" will resume deleting everything 
until the next time "Recorder-KeepCurrent" is enabled.

Putting the temporary file ("format_path") to /tmp (RAMdisk) saves CPU time,
does not harm the flash or continuously accesses the NAS (wherever the permanent
storage is).
/tmp/samba happens to be writeable for all (mpd does not run as root).

UPNP/DLNA Renderer
------------------
The upmpdcli daemon is started together with mpd. It provides a DLNA renderer.

The announced name is "fFritz".

The configuration file is /var/media/ftp/ffritz/upmpdcli.conf.

Web interface
-------------
The ympd http frontend for mpd is started at http port 82:

	http://192.168.178.1:82

Shairport-sync
==============
shairport-sync is a server implementation for the AirPort protocol.
It announces itself as "FritzBox". It will output data to
/var/tmp/shairport.fifo and has precedence over the MPD audio pipe.

shairport-sync is started via the shairport service (/nvram/ffnvram/etc/rc.d).
This script will also start the usbplayd service if required.

The configuration file is located in /nvram/ffnvram/etc/shairport-sync.conf.

If you do not want to start shairport:

	ffservice disable shairport
	ffservice stop shairport

Start script
------------

You can specify a command to be executed when a device is starting to play
("run_this_before_play_begins" option).

I use it to turn on and configure my amplifier via IR/lirc with this
script:

~~~
#!/bin/sh

irsend SEND_ONCE RAX16 AMP_POWER
usleep 100000
irsend SEND_ONCE RAX16 AMP_AUX
~~~

Bluetooth
=========
Bluetooth audio endpoint support is enabled via the bluekitchen BT stack. 
The bluetooth service runs the a2dp_sink_demo tool which reports itself
as "FritzBox" endpoint and outputs via the /tmp/bt.fifo pipe to usbplayd.

Pairing data is stored persistently in /var/media/ftp/ffritz/bt.

NFS Mounts
==========
If you want to mount an external database, use the .mtab feature as described
in README.
For example is use this entry in /var/media/ftp/ffritz/.mtab so that mpd can
access my music database on an external NAS:

    MOUNT Musik/NAS nfs://nas/Multimedia/Music -a


Remote Control with lirc
========================
Here is an example how i use irexec to use my amplifiers remote control to operate 
web radio stations via mpd/mpc:

- Create a playlist for web-radio stations, for example

	/var/media/ftp/ffritz/.mpd/playlists/radio.m3u

  (just put in the URLs of the web radion stations line by line)

- Use irrecord to create a remote control configuration file, or search for an
  existing one.

- Put the configuration file to /var/media/ftp/ffritz/etc/lirc/lircd.conf.d and 
  restart lirc (ffdaemon -R lircd).

- Edit the irexec definition file (/var/media/ftp/ffritz/etc/lirc/irexec.lircrc) to
  assign keys on the remote controll to actions.
  Below is the one i'm using.
	- The `CD_PLAY` and `PAUSE_STOP` keys are used to start/stop playing.
	- The tuner preset keys are used to go back and forth in the playlist
	- The `TUNER_ABCDE` key is used to reset the player to play the first
	  entry from the readio playlist.

```

	begin
	    prog   = irexec
	    button = CD_PLAY
	    config = mpc play
	end                  
	   
	begin
	    prog   = irexec
	    button = CD_PAUSE/STOP
	    config = mpc stop     
	end                  
	   
	begin
	    prog   = irexec
	    button = TUNER_PRESET_+
	    config = mpc next      
	end                  
	   
	begin
	    prog   = irexec
	    button = TUNER_PRESET_-
	    config = mpc prev      
	end                  
	   
	begin
	    prog   = irexec
	    button = TUNER_ABCDE
	    config = mpc crop; mpc del 1; mpc load radio; mpc play 1
	end                                    
```

- Restart irexec daemon (ffdaemon -R irexec)
