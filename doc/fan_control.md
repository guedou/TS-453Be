# Identifying Fan Controls

The QTS web interface allows to control the fan speed either automatically or manually. This document describes how to identify the QNAP binary that can be used to control the fan speed.


## Exploring the Web Interface

The first steps consists in setting the fan rotation speed manually from the QTS control panel:
![](doc/images/set_speed_manually.png)

Then, using a web browser developer console, it is possible to identify the URL controlling the fan speed. With Google Chrome, go to developer tools, in the network section, click "clear", then "record network log", click "apply" in the QNAP control panel. Look for new names, then click "stop" (aka the "record" button). Click "preview" and investigate data sent to the server. Here, `sysRequest.cgi` adjusts the fan speed:
![](doc/images/sysRequest.cgi_xml.png)

More investigations can be performed by click on the XML document and look for fan related data. The `Fan_Speed` tag contains the fan speed value selected on the scrollbar:
![](doc/images/sysRequest.cgi_xml_values.png)


## Reversing sysRequest.cgi

The `sysRequest.cgi` is a regular ELF binary than can either be retrieved from a QTS installation, or extracted from [rootfs2.bz](https://github.com/guedou/TS-453Be/blob/master/doc/qts_firmware_recovery.md).

Using [radare2](https://github.com/radareorg/radare2), let's have a look at string that contains the `fan_` keyword:
```
r2 bin/sysRequest.cgi
[0x0040d450]> izz~+fan_ # insensitive case search (~+) in strings
488 0x00007205 0x00407205  18  19 (.dynstr) ascii SE_Is_TFAN_Support
490 0x00007229 0x00407229  21  22 (.dynstr) ascii SE_Get_FAN_Region_Num
491 0x0000723f 0x0040723f  35  36 (.dynstr) ascii SE_Get_FAN_Region_Scrollbar_Support
492 0x00007263 0x00407263  39  40 (.dynstr) ascii SE_Get_FAN_Region_TFan_Optional_Support
4750 0x00052665 0x00452665  15  16 (.rodata) ascii fan_num_support
4751 0x00052675 0x00452675   8   9 (.rodata) ascii Fan_Mode
4754 0x00052693 0x00452693   9  10 (.rodata) ascii tfan_mode
4759 0x000526c4 0x004526c4   9  10 (.rodata) ascii Fan_Speed
4760 0x000526ce 0x004526ce  12  13 (.rodata) ascii cpu_fan_mode
4773 0x00052796 0x00452796  13  14 (.rodata) ascii cpu_fan_speed
5290 0x00054318 0x00454318  12  13 (.rodata) ascii CPU_Fan_type
5291 0x00054325 0x00454325  15  16 (.rodata) ascii CPU_Fan_actions
5296 0x00054374 0x00454374  13  14 (.rodata) ascii CPU_Fan_speed
5717 0x00057850 0x00457850  75  76 (.rodata) ascii /sbin/hal_event --se_set_tfan_mode_smart_intelligent fan_region=%d,tmode=%d
5718 0x000578a0 0x004578a0  65  66 (.rodata) ascii /sbin/hal_event --se_set_fan_mode_smart_intelligent fan_region=%d
5722 0x00057a58 0x00457a58 106 107 (.rodata) ascii /sbin/hal_event --se_set_fan_mode_smart_custom unit=%d,stop_temp=%d,low_temp=%d,high_temp=%d,fan_region=%d
5723 0x00057ac8 0x00457ac8  65  66 (.rodata) ascii /sbin/hal_event --se_set_fan_mode_fixed fan_level=1 fan_region=%d
5724 0x00057b10 0x00457b10  65  66 (.rodata) ascii /sbin/hal_event --se_set_fan_mode_fixed fan_level=4 fan_region=%d
5725 0x00057b58 0x00457b58  65  66 (.rodata) ascii /sbin/hal_event --se_set_fan_mode_fixed fan_level=7 fan_region=%d
5727 0x00057bd8 0x00457bd8  70  71 (.rodata) ascii /sbin/hal_event --se_set_fan_scrollbar_stepping fan_region=%d,value=%d
```

The strings that contains `/sbin/hal_event` are promising. Indeed, on QTS, the following command will make the fan run at full speed `/sbin/hal_event --se_set_fan_scrollbar_stepping fan_region=0,value=100` while `/sbin/hal_event --se_set_fan_scrollbar_stepping fan_region=0,value=10` will stop it.


## Running QNAP in a chroot

The `hal_event` can be run on a regular Ubuntu installation. It is a really good news that indicates that no kernel modules is required to interact with fan controls. From a QTS installation, you need to retrieve `hal_event`, `hal_daemon`, `hal_tool`, their corresponding shared libraries as well as configuration information with `hal_tool dump`.

Then, you need to create the chroot using the following commands:
```
mkdir qnap/
cd qnap/
mount -t proc proc proc/
mount -t sysfs sys sys/
mount -o bind /dev dev/
mount -o bind /dev/pts dev/pts
# copy the three binaries, the libraries, and extract the tarball created with hal_dmp (*_TS-453Be_dump.tgz)
cd ..
chroot qnap/
```

Finally, launch `/sbin/hal_daemon -f` and you should be able to use `hal_event`. 


## Other Useful QTS Command Line Tools

Display chassis information:
```
hal_app --se_getinfo=0
========================================================================================================
enc_id enc_parent_sys_id enc_sys_id wwn vendor model status protocol disk_no
fan_no cpu_fan_no temp_no gpio_no
0       root    root            QNAP    TS-453Be        0       0       4
1       0       0       0       2
========================================================================================================
```

Retrieve temperatures:
```
getsysinfo cputmp
34 C/93 F

getsysinfo systmp 1 
22 C/72 F

get_hd_temp 2        
26
```

Retrieve the number of fans:
```
getsysinfo sysfannum
1
```

Display information:
```
hal_tool info
Current Time: 2019-01-04 14:21:52
Model Name:   TS-453Be
Mac Address:  [eth0] 24:5e:be:22:42:7f
Firmware:     TS-X53B_20181114-4.3.5.0760.img
Kernel:       linux-4.2.8 x86_64
BIOS Version: QY47AR54
EC Version:   QY47EF16
Model Config: /etc/model.conf -> /etc/model_QY472_QY580_15_11.conf
MB VPD:       70-0QY472150
BP VPD:       70-1QY580110
IP Info:      [qvs0] 24:5e:be:22:42:7f (192.168.28.7) -> [gateway] 192.168.28.1
```

Note: the file `/etc/model_QY472_QY580_15_11.conf` is extracted by `hal_dump`.

`hal_tool` is a shell script that uses the following commands:
```
get_nas_info()
{
HAL_MODEL_NAME=$(/sbin/getcfg "System Enclosure" "MODEL" -d "TS-XXX" -f
/etc/model.conf)
FW_MODEL=$(/sbin/getcfg "System" "Model")
FW_VERSION=$(/sbin/getcfg "System" "Version")
FW_VERNUM=$(/sbin/getcfg "System" "Number")
FW_BUILD=$(/sbin/getcfg "System" "Build Number")
MODEL_SL=$(/bin/ls -al /etc/model.conf)
VPD04=$(/sbin/hal_app --vpd_get_field obj_index=0:4)
VPD17=$(/sbin/hal_app --vpd_get_field obj_index=1:7)
BIOS=$(/sbin/dmidecode --string bios-version 2>/dev/null)
EC=$(/sbin/hal_app --get_ec_version)
MCU=$(/sbin/hal_app --get_mcu_version mode=0 | /bin/awk {'print $4'})
IP=$(/sbin/hal_tool ip)
/bin/echo "Current Time: $(/bin/date '+%F %T')"
/bin/echo "Model Name:   ${HAL_MODEL_NAME}"
/bin/echo "Mac Address:  [eth0] $(/bin/cat /sys/class/net/eth0/address)"
/bin/echo "Firmware:     ${FW_MODEL}_${FW_BUILD}-${FW_VERSION}.${FW_VERNUM}.img"
/bin/echo "Kernel:       linux-$(/bin/uname -r) $(/bin/uname -m)"
[ "x${BIOS}" = "x" ] || /bin/echo "BIOS Version: ${BIOS}"
[ "x${EC}" = "x" ] || /bin/echo "EC Version:   ${EC}"
[ "x${MCU}" = "x" ] || /bin/echo "MCU Version:  ${MCU}"
/bin/echo "Model Config: /etc/model.conf -> ${MODEL_SL##*-> }"
/bin/echo "MB VPD:       ${VPD04##*= }"
/bin/echo "BP VPD:       ${VPD17##*= }"
/bin/echo "IP Info:      ${IP}"
}
```
