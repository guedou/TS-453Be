# QNAP - change OS

## Alternatives solutions

- install Ubunu on the DOM
- grub chainloading
- copy kernel & initrd

### BELOW IS TO CHECK ###

- boot with Ubuntu on USB; front port; press F7 to select the USB key
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

## grub

- modify QNAP grub.cfg and copy kernel and initrd to QNAP partitions
- chainloading
- load another grub.cfg


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

## Installation

- regular Ubuntu installation on the disk/disks
- copy kernel & initrd to the DOM (after removing files)
- edit grub.cfg on the DOM & reboot
