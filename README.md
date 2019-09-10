# Linux on QNAP TS-453Be
[![Twitter Follow](https://img.shields.io/twitter/follow/guedou.svg?style=social)](https://twitter.com/intent/follow?screen_name=guedou)

This repository provides notes to install Ubuntu on a [QNAP TS-453Be NAS](https://www.qnap.com/en/product/ts-453be), and to understand how some [QNAP QTS](https://www.qnap.com/qts/) binaries work.

 The included code allows to interact with the IT8528
Super I/O controller in order to control the fan speed, and read the chassis
temperature.

Why?
- [QNAP Privacy Policy](https://www.qnap.com/en/before_buy/con_show.php?op=showone&cid=17) indicates that QNAP `may collect your activities on our website, cloud services, software, and hardware`
- [many CVEs](https://www.qnap.com/en/security-advisory) targeting QTS and QNAP softwares
- [The eCh0raix Ransomware](https://www.anomali.com/blog/the-ech0raix-ransomware)
- hardcoded QTS limitations with 4GB of RAM
- not enough control on encrypted data, and restoration without QTS
- few way to enforce firewall rules


## panql


## Installing Ubuntu

- modify QNAP grub.cfg and copy kernel and initrd to QNAP partitions
- chainloading
- load another grub.cfg

- regular Ubuntu installation on the disk/disks
- copy kernel & initrd to the DOM (after removing files)
- edit grub.cfg on the DOM & reboot

MAYBE needed:
```
root@kotatsu:/media# grub-editenv ./grubenv list
saved_entry=0
prev_saved_entry=0
```


## Reversing QTS binaries

https://sourceforge.net/projects/qosgpl/


## Similar Attempts

To my knowledge, others have succeeded in replacing QTS with another Linux distribution:
- http://www.cyrius.com/debian/orion/qnap/
- https://github.com/vanschelven/qnap-x53
- https://wiki.qnap.com/wiki/Debian_Installation_On_QNAP
