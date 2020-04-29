Notes on first-time installation
================================

Checking branding
-----------------

If you run a branded box, the (modified) retail images will not work.
This can be be patched in the firmware image by just ignoring the firmware_version
variable. There is some information in the IPPF forum on how to do this if you
look for it.

Completely removing the branding is possible, very dangerous and i will not document it.

Checking BIOS version
---------------------

Before deciding/attempting how/whether to modify the box, check which BIOS version
you are running. To do so, restart the box and obtain extended support data
(erweiterete supportdaten) via http://192.168.178.1/support.lua.

It is generally a good idea to KEEP this file since it might become helpful
for recovering a bricked box!

Once the .txt file is downloaded look for the BIOS strings as listed below:

For BIOS < CGM2.86C.627075.R.1910091149 10/09/2019 - Update via serial console
------------------------------------------------------------------------------

This requires opening the box and connecting a UART adapter. Unless you break the
box by doing so, this method is relatively safe since it uses the standard AVM
update method.

Refer to https://www.ip-phone-forum.de/threads/fb-6591-verschiedenes.303332/post-2342169

Update via EVA (BIOS CGM2.86C.627075.R.1910091149 10/09/2019)
-------------------------------------------------------------

This is the more or less well known method via the EVA bootloader, using a ftp client
to push the four required eMMC partition contents.

There might be LATER BIOS VERSION .. i can't tell if/how it works there, so check twice
before proceeding!

Required is a ftp client which supports passive mode. Also recommended is a switch between
your PC and the box to avoid the link going down when the box restarts. And/or the IP address
of your host on the box network (192.168.178) should be configured statically.

1. Generate the update image as described in README.md. 

   Once complete, there will be some image files in the subdirectory tmp/uimage:

        ffritz$ cd tmp/uimage/
        ffritz/tmp/uimage$ ls -l
        -r--r--r-- 1 felix osboxes  9437184 Apr 13 07:37 part_02_ATOM_KERNEL.bin
        -rw-r--r-- 1 felix osboxes 29569024 Apr 13 07:37 part_03_ATOM_ROOTFS.bin
        -r--r--r-- 1 felix osboxes  2335056 Apr 13 07:36 part_08_ARM_KERNEL.bin
        -rw-r--r-- 1 felix osboxes 17981440 Apr 13 07:37 part_09_ARM_ROOTFS.bin

2. Repower the box and connect via ftp client after ca. 5 seconds:

        ffritz/tmp/uimage$ ftp 192.168.178.1

   user/password is adam2/adam2

3. Configure ftp (binary/pasive)

        bin
        quote MEDIA FLSH
        passive

4. Determine the current boot partition set:

        quote GETENV linux_fs_start

   This will provide the currently active partition set (0 or 1). We always
   want to write the one which is not active, and switch to it afterwards.

   The next steps, if done wrong, can overwrite the eMMC in a way that the
   box can no longer start!
   The write operations (especially part_03_ATOM_ROOTFS.bin) take some
   minutes, so be patient.

   To write and switch to partition set I=0 (if linux_fs_start = 1):

        put part_03_ATOM_ROOTFS.bin mtd0
        put part_02_ATOM_KERNEL.bin mtd1
        put part_09_ARM_ROOTFS.bin mtd6
        put part_08_ARM_KERNEL.bin mtd7
        quote SETENV linux_fs_start 0

   To write and switch to partition set I=1 (if linux_fs_start = 0):

        put part_03_ATOM_ROOTFS.bin mtd;
        put part_02_ATOM_KERNEL.bin mtd<
        put part_09_ARM_ROOTFS.bin mtd=
        put part_08_ARM_KERNEL.bin mtd>
        quote SETENV linux_fs_start 1

5. Reboot

        quote REBOOT

   The modified image should now start, and telnet/ssh login should be possible as described
   in README.md. Subsequent updates can be done using the burnuimg tool.

   If something went wrong, the box will either automatically switch back to the old boot bank,
   or you need to change back linux_fs_start manually, or ....

FB6591 eMMC partition information
=================================

P: Corresponds to /dev/mmcblk0pXX devices
I: Corresponds to linux_fs_start content

~~~
P  I Content           Type         Start End    Size   Start    End
                                   LBA   LBA    LBA    Offset   Offset     EVA mtd
-- - ---------------- ------------  ----- ------ ------ -------- --------- -------
1  0 SIGBLOCK0        raw          800   8FF    100    100000   11FE00
2  0 APP_CPU_KERNEL0  uefi/fat     1000  57FF   4800   200000   AFFE00     mtd1
3  0 APP_CPU_ROOTFS0  squashfs     5800  297FF  24000  B00000   52FFE00    mtd0
4  0 NP_CPU_KERNEL0   raw_bzImage  29800 2BFFF  2800   5300000  57FFE00    mtd7
5  0 NP_CPU_ROOTFS0   squashfs_be  2C000 357FF  9800   5800000  6AFFE00    mtd6
6  0 GW_FS0           tar          35800 497FF  14000  6B00000  92FFE00
7  1 SIGBLOCK1        raw          49800 498FF  100    9300000  931FE00
8  1 APP_CPU_KERNEL1  uefi/fat     4A000 4E7FF  4800   9400000  9CFFE00    mtd<
9  1 APP_CPU_ROOTFS1  squashfs     4E800 727FF  24000  9D00000  E4FFE00    mtd;
10 1 NP_CPU_KERNEL1   raw_bzImage  72800 74FFF  2800   E500000  E9FFE00    mtd>
11 1 NP_CPU_ROOTFS1   squashfs_be  75000 7E7FF  9800   EA00000  FCFFE00    mtd=
12 1 GW_FS1           tar          7E800 927FF  14000  FD00000  124FFE00
13   APP_CPU_NVRAM    ext4         92800 967FF  4000   12500000 12CFFE00
14   NP_CPU_NVRAM     ext4         96800 9A7FF  4000   12D00000 134FFE00
15   AVM_TFFS1        tffs         9A800 9E7FF  4000   13500000 13CFFE00   could be mtd2
16   AVM_TFFS2        tffs         9E800 A27FF  4000   13D00000 144FFE00
17   AVM_UPD_TMP      ???          A2800 E27FF  40000  14500000 1C4FFE00
18   AVM_MEDIA        ext4         E2800 75FFDE 67D7DF 1C500000 EBFFBC00
~~~
