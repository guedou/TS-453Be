# QTS Firmware Recovery

QNAP provides [useful information](https://wiki.qnap.com/wiki/Firmware_Recovery#Firmware_Recovery_Guide_for_TS-x53B_Series.2C_TS-x53BU_Series.2C_TS-x51A_Series.2C_TS-x53U_series.2C_TS-x53A_Series.2C_TBS-453A.2C_TVS-x63_Series.2C_TVS-x63.2B_Series.2C_TVS-x71_Series.2C_TVS-x71U_Series.2C_TVS-x72XT_Series.2C_TVS-x72XU_Series.2C_TVS-x73_Series.2C_TS-x73U_Series.2C_TS-x77_Series.2C_TS-1685.2C_TVS-ECx80_.2F_TVS-ECx80.2B.2C_TVS-ECx80U.C2.A0Series.2C_TVS-x82_Series.2C_TVS-x82T_Series.2C_TVS-x82ST_Series.2C_TVS-882BR.2C_TVS-882BRT3.2C_TVS-951X.2C_TVS-1282T3.2C_TVS-1582TU.2C_TDS-16489U.2C_TDS-16489U_R2.2C_TES-x85U_series_NAS) to flash the TS-453Be DOM with a `System Full Image`. In a nutshell, you need to download http://eu1.qnap.com/Storage/tsd/fullimage/F_TS-X53B_20161212-1.3.0_EFI.img, copy it to a bootable USB drive, and simply do `cp F_TS-X53B_20161212-1.3.0_EFI.img /dev/mmcblk1`. The section below describes how to explore the image content and access QNAP binaries and libraries.

Notes:
- the file SHA256 digest changes from one download to another
- there is plenty of space on the DOM to store other things as the recovery image is only 500MB


## Mount QNAP Image Partitions

First, let's list partitions:
```
parted F_TS-X53B_20161212-1.3.0_EFI.img 'unit b' print
WARNING: You are not superuser.  Watch out for permissions.
Model:  (file)
Disk F_TS-X53B_20161212-1.3.0_EFI.img: 515899392B
Sector size (logical/physical): 512B/512B
Partition Table: gpt
Disk Flags: 

Number  Start       End         Size        File system  Name              Flags
 1      1048576B    17825791B   16777216B   fat16        EFI System        boot, esp
 2      17825792B   252706815B  234881024B  ext2         Linux filesystem
 3      252706816B  487587839B  234881024B  ext2         Linux filesystem
 5      487587840B  495976447B  8388608B    ext2         Linux filesystem
 6      495976448B  504365055B  8388608B    ext2         Linux filesystem
```

For example, you can mount the first partition with:
```
mount -o loop,offset=17825792 F_TS-X53B_20161212-1.3.0_EFI.img ./fw_recovery/partition_1/
```

Here are the content of the partitions:
1. grub.cfg (the one that is edited during the Ubuntu installation) and GRUB modules
2. Linux kernel images, initrd & QNAP software (qpkg.tar, rootfs2.bz, rootfs_ext.tgz)
3. same as #2 but for rescue
5. empty
6. empty


## Exploring the initrd content

```
mkdir initrd_content
cd initrd_content
xz -dc ../partition_2/boot/initrd.boot | cpio --quiet -i --make-directories
tree -L 1
.
├── 10g.sh
├── bin
├── dev
├── etc
├── init
├── lib
├── lib64 -> lib
├── linuxrc -> bin/busybox
├── lost+found
├── mnt
├── opt
├── php.ini -> /etc/config/php.ini
├── proc
├── root
├── sbin
├── share
├── tmp
└── var

14 directories, 4 files
```

Note: the file `lib/libuLinux_hal.so` can be used to reverse the function that communicates with the ITE IT8528 EC.


## Exploring the qpkg.tar content

```
mkdir qpkg.tar_content
cd qpkg.tar_content
tar xvf ../partition_2/boot/qpkg.tar -C .
tree
.
├── antivirus.tgz
├── avahi0630.tgz
├── bluetooth.tgz
├── chassisView.tgz
├── helpdesk.bin
├── ImageMagick.tgz
├── jsLib.tgz
├── language.tgz
├── ldap_server.tgz
├── libboost.tgz
├── mariadb5.tgz
├── medialibrary.tgz
├── mt-daapd.tgz
├── mtpBinary.tgz
├── netmgr.bin
├── pkg_flag
├── Python.tgz
├── qcli.tgz
├── QcloudSSLCertificate.bin
├── QsyncServer.bin
├── radius.tgz
├── ResourceMonitor.bin
├── samba4.tgz
├── Samples.tgz
├── textEditor.tgz
├── vim.tgz
└── wifi.tgz

0 directories, 27 files
```


## Exploring the rootfs2.bz content

```
mkdir rootfs2.bz_content
cd rootfs2.bz_content/
bunzip2 --stdout ../partition_2/boot/rootfs2.bz > rootfs2
tar xvf rootfs2 -C .
tree -L 2
.
├── home
│   ├── httpd
│   └── Qhttpd
├── lib
│   └── modules
├── rootfs2
└── usr
    ├── bin
    ├── etc
    ├── lib
    ├── libexec
    ├── local
    ├── man
    ├── sbin
    └── share

14 directories, 1 file
```


## Exploring the rootfs_ext.tgz content

```
mkdir rootfs_ext.tgz_content
cd rootfs_ext.tgz_content
tar xzvf ../partition_2/boot/rootfs_ext.tgz 
mkdir rootfs_ext.img_content
sudo mount -o loop rootfs_ext.img rootfs_ext.img_content
tree rootfs_ext.img_content/
rootfs_ext.img_content/
├── addon_flag
├── debug_flag
├── lost+found
└── opt
    └── source
        └── apache_php5.tgz

3 directories, 3 files
```
