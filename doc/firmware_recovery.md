``
https://wiki.qnap.com/wiki/Firmware_Recovery
```

## Mount QNAP image partitions

List partitions
```
parted F_TS-X53B_20161212-1.3.0_EFI.img 'unit b' print
WARNING: You are not superuser.  Watch out for permissions.
Model:  (file)
Disk /home/guedou/Projects/qnap/F_TS-X53B_20161212-1.3.0_EFI.img: 515899392B
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

Mount the first partition:
```
mount -o loop,offset=17825792 F_TS-X53B_20161212-1.3.0_EFI.img /media/
```

Partitions content:
1 - grub.cfg and modules
2 - Linux kernel images, initrd & QNAP software
3 - same as #2 but for rescue
5 - empty
6 - empty

## Explore the initrd content

```
mkdir initrd_content
xz -dc ../partition_2/boot/initrd.boot | cpio --quiet -i --make-directories
```

## Extract

```
sha256sum --check <(echo 83aab9b5e23a0385a919d651f47f45ebe6348e61c8514d18e28070ae4dce61c7  F_TS-X53B_20161212-1.3.0_EFI.img)
mkdir ./mnt
sudo mount --options loop,offset=17825792 F_TS-X53B_20161212-1.3.0_EFI.img ./mnt
bunzip2 mnt/boot/rootfs2.bz --stdout > rootfs2
sudo umount ./mnt
```

## sysRequest.cgi

```
bunzip partition_2/boot/rootfs2.bz
tar tvf rootfs2 |grep sysRequest.cgi
-rwxr-xr-x root/root      404016 2016-12-12 05:22 home/httpd/cgi-bin/sys/sysRequest.cgi
lrwxrwxrwx root/root           0 2016-12-12 05:22 home/httpd/cgi-bin/sys/sysStandard.cgi -> sysRequest.cgi
lrwxrwxrwx root/root           0 2016-12-12 05:22 home/httpd/cgi-bin/sys/syslogView.cgi -> sysRequest.cgi
```


