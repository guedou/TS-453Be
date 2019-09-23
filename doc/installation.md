# Installing Ubuntu

The TS-453Be NAS boots from a [4GB eMMC](https://www.techpowerup.com/review/qnap-ts453b/4.html) soldered to the main board. Different methods can be used to install Ubuntu on the NAS:
- on the DOM, like done on the [TS-459 Pro](https://jorgbosman.nl/QNAP_TS-459_Pro_with_Ubuntu)
- on the disks, and chainload Ubuntu grub from the DOM grub
- on the disks, modify the DOM `grub.cfg` and copy the Ubuntu kernel and initrd to QNAP partitions

Unfortunately, it is not possible to permanently boot the NAS from an USB drive.

![QNAP DOM](doc/images/in_thgbmdg5_ve4543.jpg)

Note: if the installation goes wrong the DOM can be easily reflashed using the following instructions https://wiki.qnap.com/wiki/Firmware_Recovery

This tutorial uses the third method to keep DOM modifications as small as possible. In a nutshell, it consists in:
- a regular Ubuntu installation on SATA disks
- copying kernel & initrd file sto the DOM (after removing the original QTS ones)
- editing `grub.cfg` on the DOM & reboot

## Steps

- create a bootable Ubuntu USB drive
- boot with Ubuntu on the USB front port
- press F7 to select the USB drive
- install Ubuntu server on the SATA disks
- copy the kernel


### BELOW IS TO CHECK ###

- edit grub.cfg
  - boot on couette HD: fail
  - Ubuntu installed on sda: fail
- force boot on USB: fail
-  keep the qnap kernel and
  - change the uuid: fail
  - change fstab in initrd: fail
—
- initrd avec « DEBUG »
- copy the Ubuntu kernel to the DOM: works
  - root=/dev/sda1
  - boot sur les entrées grub qnap
- install Ubuntu in the DOM

—

- flasher avec l’image QNAP
- installer un serveur Ubuntu en RAID1
- copier le noyau mettre à jour grub.cfg

### STEPS BELOW


## install UEFI on the Ubuntu disk

```
(parted) toggle 1 esp
(parted) p                                                                
Model: ATA WDC WD20EFRX-68E (scsi)
Disk /dev/sda: 2000GB
Sector size (logical/physical): 512B/4096B
Partition Table: msdos
Disk Flags: 

Number  Start   End    Size    Type     File system     Flags
 1      1049kB  200MB  199MB   primary  fat32           boot, esp
 2      200MB   300GB  300GB   primary  ext4
 3      300GB   308GB  8000MB  primary  linux-swap(v1)
```

See https://wiki.debian.org/GrubEFIReinstall
```
apt-get install --reinstall grub-efi
grub-install /dev/sda
```

Note:
QNAP OS seems to reinstall itself on the QT_BOOT_PART{2,3} partitions ...


## RAID1

- partition with type 0xFD
- mdadm --create --verbose --level=mirror /dev/md1 --raid-devices=2 --force /dev/sda4 /dev/sdb4
- cryptsetup luksFormat /dev/md1
- cryptsetup luksOpen /dev/md1 data
- mkfs.ext4 /dev/mapper/data

(see https://blog.tinned-software.net/automount-a-luks-encrypted-volume-on-system-start/)
- edit /etc/crypttab and the device and the key to use
- edit /etc/fstab

Notes:
  - /dev/sdc & /sdd are used for / & /home
  - remove old md device with `mdadm --manage --stop /dev/md127`

## FIND A TITLE

MAYBE needed:
```
root@kotatsu:/media# grub-editenv ./grubenv list
saved_entry=0
prev_saved_entry=0
```
