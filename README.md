# Linux on QNAP TS-453Be
[![Twitter Follow](https://img.shields.io/twitter/follow/guedou.svg?style=social)](https://twitter.com/intent/follow?screen_name=guedou)

This repository provides notes to install Ubuntu on a [QNAP TS-453Be NAS](https://www.qnap.com/en/product/ts-453be), and to understand how some [QNAP QTS](https://www.qnap.com/qts/) features work. The [panq](https://github.com/guedou/TS-453Be/blob/master/panq/) binary allows to interact with the ITE IT8528 Super I/O controller in order to control the fan speed, the front LED and read the chassis temperature. This EC is not supported by regular Linux power management tools.

![TS-453Be](https://github.com/guedou/TS-453Be/blob/master/doc/images/313_1516332261_TS-453Be_Left-angle-of-elevation.png)

## Why?

From memory, I did this project to:
1. learn new things, and push my limits
2. bypass QTS restrictions: hard-coded virtualisation limitation with 4GB of RAM, few control on encrypted data, not enough confidence to restore data without QTS, few ways to enforce firewall rules...
3. get rid of the [many CVEs](https://www.qnap.com/en/security-advisory) targeting QTS and QNAP software
4. avoid the [QNAP Privacy Policy](https://www.qnap.com/en/before_buy/con_show.php?op=showone&cid=17) that indicates that QNAP `may collect your activities on our website, cloud services, software, and hardware`
5. stay away of the [The eCh0raix Ransomware](https://www.anomali.com/blog/the-ech0raix-ransomware)


## Documentation

- [installing Ubuntu](https://github.com/guedou/TS-453Be/blob/master/doc/installation.md)
- [identifying fan controls](https://github.com/guedou/TS-453Be/blob/master/doc/fan_control.md)
- [reversing QNAP binaries](https://github.com/guedou/TS-453Be/blob/master/doc/reversing.md)


## Similar Attempts

To my knowledge, others have succeeded in replacing QTS with another Linux distribution:
- https://jorgbosman.nl/QNAP_TS-459_Pro_with_Ubuntu
- http://www.cyrius.com/debian/orion/qnap/
- https://github.com/vanschelven/qnap-x53
- https://wiki.qnap.com/wiki/Debian_Installation_On_QNAP
