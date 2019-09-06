FB6591 eMMC partition information
=================================

1st column corresponds to /dev/mmcblk0pXX devices

~~~
# Content           Type         Start End    Size   Start    End
                                 LBA   LBA    LBA    Offset   Offset
-- ---------------- ------------  ----- ------ ------ -------- ---------
1  SIGBLOCK0        raw          800   8FF    100    100000   11FE00  
2  APP_CPU_KERNEL0  uefi/fat     1000  57FF   4800   200000   AFFE00
3  APP_CPU_ROOTFS0  squashfs     5800  297FF  24000  B00000   52FFE00
4  NP_CPU_KERNEL0   raw_bzImage  29800 2BFFF  2800   5300000  57FFE00
5  NP_CPU_ROOTFS0   squashfs_be  2C000 357FF  9800   5800000  6AFFE00
6  GW_FS0           tar          35800 497FF  14000  6B00000  92FFE00
7  SIGBLOCK1        raw          49800 498FF  100    9300000  931FE00
8  APP_CPU_KERNEL1  uefi/fat     4A000 4E7FF  4800   9400000  9CFFE00
9  APP_CPU_ROOTFS1  squashfs     4E800 727FF  24000  9D00000  E4FFE00
10 NP_CPU_KERNEL1   raw_bzImage  72800 74FFF  2800   E500000  E9FFE00
11 NP_CPU_ROOTFS1   squashfs_be  75000 7E7FF  9800   EA00000  FCFFE00
12 GW_FS1           tar          7E800 927FF  14000  FD00000  124FFE00
13 APP_CPU_NVRAM    ext4         92800 967FF  4000   12500000 12CFFE00
14 NP_CPU_NVRAM     ext4         96800 9A7FF  4000   12D00000 134FFE00
15 AVM_TFFS1        tffs         9A800 9E7FF  4000   13500000 13CFFE00
16 AVM_TFFS2        tffs         9E800 A27FF  4000   13D00000 144FFE00
17 AVM_UPD_TMP      ???          A2800 E27FF  40000  14500000 1C4FFE00
18 AVM_MEDIA        ext4         E2800 75FFDE 67D7DF 1C500000 EBFFBC00
~~~
