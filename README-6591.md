FB6591 eMMC partition information
=================================

P: corresponds to /dev/mmcblk0pXX devices
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
8  1 APP_CPU_KERNEL1  uefi/fat     4A000 4E7FF  4800   9400000  9CFFE00	   mtd<
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

Update via EVA (as of BIOS CGM2.86C.627075.R.1910091149 10/09/2019)
===================================================================

!! DANGER HERE, try on your own risk !!

~~~
bin
quote MEDIA FLSH
passive
~~~

To write partition set I=0
--------------------------
~~~
put part_03_ATOM_ROOTFS.bin mtd0
put part_02_ATOM_KERNEL.bin mtd1
put part_09_ARM_ROOTFS.bin mtd6
put part_08_ARM_KERNEL.bin mtd7
~~~

To write partition set I=1
--------------------------
~~~
put part_03_ATOM_ROOTFS.bin mtd;
put part_02_ATOM_KERNEL.bin mtd<
put part_09_ARM_ROOTFS.bin mtd=
put part_08_ARM_KERNEL.bin mtd>
~~~
