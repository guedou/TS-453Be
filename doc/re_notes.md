modprobe it87 force_id=0x8712

gives
[98410.649245] it87: Device not activated, skipping
[98410.649371] it87: Found IT8712F chip at 0xfff8, revision 15
[98410.649397] it87: VID is disabled (pins used for GPIO)
[98410.649433] it87: Beeping is supported

The IT8712F support is modified by QNAP in their kernel source.

sudo isadump 0x2E 0x2F

EC = Embedded Controller

https://stackoverflow.com/questions/24484193/fintek-f71869a-gpio-control

http://www.rom.by/files/it8728f_datasheet.pdf
https://www.usbid.com/datasheets/usbid/2000/2000-q3/pc87393_bios.pdf

r2 hal_event
aar
axt @ 0x43d087
sym.imp.SE_Get_Capability

[0x0040a270]> izz |grep fan_init
3337 0x0003b2f0 0x0043b2f0  17  18 (.rodata) ascii ec_fan_initialize
[0x0040a270]> axt @ 0x0043b2f0
sub.SE_Get_Info_af0 0x421ea7 [data] mov edx, str.ec_fan_initialize

this function calls sym.imp.SE_Get_Info

In `main()`
- 0x658dc0 is an array containing long option names addresses every 32 bytes:
```
:> ps @ `pv @ 0x658dc0 + 32*0`
clear_ipc
:> ps @ `pv @ 0x658dc0 + 32*1`
retrieve_booting_event
:> ps @ `pv @ 0x658dc0 + 32*7`
se_set_fan_scrollbar_stepping
```
Display all options:
```
pv @@= `?s 0x658dc0 0x658dc0+32*43 32` > /tmp/offsets.txt 
ps @@./tmp/offsets.txt
```

- 0x443b40 cointains pointers to function that handles the options
```
pd 42 @ `pv @ 0x443b40 + 8*7` # handles the se_set_fan_scrollbar_stepping option
```

The function 0x443b40 seems to retrieve a message queue ID from a file, and call
msgsnd. The corresponding message ID could be 0x108.

By the way,
```
md5sum bin/hal_*
be28da49b57ef4690826fa8dde3a3a5b  bin/hal_daemon
be28da49b57ef4690826fa8dde3a3a5b  bin/hal_event
```

## QNAP chroot


cd qnap/
mount -t proc proc proc/
mount -t sysfs sys sys/
mount -o bind /dev dev/
mount -o bind /dev/pts dev/pts
cd ..
chroot qnap/

## Useful commands

```
getsysinfo cputmp
34 C/93 F

getsysinfo sysfannum
1

getsysinfo systmp 1 
22 C/72 F
```

Works fine without hal_daemon !

```
get_hd_temp 2        
26
```

```
# hal_tool dump
142145_2019-01-04_TS-453Be_dump.tgz saved.
```

```
hal_tool info
Current Time: 2019-01-04 14:21:52
Model Name:   TS-453Be
Mac Address:  [eth0] 24:5e:be:22:47:7f
Firmware:     TS-X53B_20181114-4.3.5.0760.img
Kernel:       linux-4.2.8 x86_64
BIOS Version: QY47AR54
EC Version:   QY47EF16
Model Config: /etc/model.conf -> /etc/model_QY472_QY580_15_11.conf
MB VPD:       70-0QY472150
BP VPD:       70-1QY580110
IP Info:      [qvs0] 24:5e:be:22:47:7f (192.168.42.7) -> [gateway] 192.168.42.1
```

This is shell script calling:
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

## Command used to manually set the fan speed

These commands were identified by look at the POST HTTP commands send to the
server from the web interface, and the strings contained in the related cgi.

[~] # /sbin/hal_event --se_set_fan_scrollbar_stepping fan_region=0,value=73
[~] # /sbin/hal_event --se_set_fan_scrollbar_stepping fan_region=0,value=10

`/sbin/hal_daemon -f` must run

[~] # hal_app --se_getinfo=0
========================================================================================================
enc_id enc_parent_sys_id enc_sys_id wwn vendor model status protocol disk_no
fan_no cpu_fan_no temp_no gpio_no
0       root    root            QNAP    TS-453Be        0       0       4
1       0       0       0       2
========================================================================================================

### RE getsysinfo

af print_help 0x004016a0

sysfan processing is done in block 0x401adc. The call to the function that reads
fan speed is done in 0x401af9.

The function at 0x40155a calls sym.imp.SE_Get_System_Status and prints the speed
with the following format string "%d RPM" (100% ~1800 RPM;

The SE_Get_System_Status functions reads information from /tmp/em/em_0.info !
This file change every minute and is likely written by hal_daemon

Identifying SET_SCROLLBAR_STEPPING string in hal_event binary:
```
[0x0040c380]> izz~STEP
4366 0x0004b480 0x0044b480  79  80 (.rodata) ascii %s(%d):ENC
SET_SCROLLBAR_STEPPING got called,fan region[%d], fan stepping = %d\n
[0x0040c380]> axt @ 0x0044b480
(nofunc) 0x430732 [DATA] mov esi, str.s__d_:ENC_SET_SCROLLBAR_STEPPING_got_called_fan_region__d___fan_stepping____d
```

### RE libs

x86-64 calling convention: RDI, RSI, RDX, RCX, R8, R9, XMM0–7

The IT8528 string is used in a function in libuLinux_hal.so
```
[0x0007f896]> izz~8528
8528 0x0009cf21 0x0009cf21   4   5 (.text) ascii D$XA
13432 0x000e9fbb 0x000e9fbb   6   7 (.rodata) ascii IT8528
[0x0007f896]> axt @ 0x000e9fbb sym.se_sys_check_ec_support
0x7f896 [DATA] lea rsi, str.IT8528
```

This function reads /etc/model.conf (linked to
/etc/model_QY472_QY580_15_11.conf) and checks that the SIO_DEVICE key in the
`[System Enclosure]` section contains `IT8528`.

This function seems to be used a lot:
```
[0x00029880]> afl~se_sys_check_ec_support
0x00028f00    1 6            sub.se_sys_check_ec_support_24_f00
0x0007f860    4 89           sym.se_sys_check_ec_support
[0x00029880]> axt @ sym.se_sys_check_ec_support
[0x00029880]> axt @ sub.se_sys_check_ec_support_24_f00
sym.HAL_MB_Is_EC_Support 0x45721 [CALL] call sub.se_sys_check_ec_support_24_f00
sym.se_sys_temperature_calibrate 0x7f907 [CALL] call sub.se_sys_check_ec_support_24_f00
sym.se_sys_cpu_temp_feedback 0x7f982 [CALL] call sub.se_sys_check_ec_support_24_f00
sym.se_sys_set_bbu_test_mode 0x7f9d4 [CALL] call sub.se_sys_check_ec_support_24_f00
sym.se_sys_init_audio_board 0x7fbb5 [CALL] call sub.se_sys_check_ec_support_24_f00
sym.se_sys_get_ec_version 0x7fc40 [CALL] call sub.se_sys_check_ec_support_24_f00
sym.se_sys_init_temp 0x7fcae [CALL] call sub.se_sys_check_ec_support_24_f00
sym.se_sys_init_temp_offset 0x7fd24 [CALL] call sub.se_sys_check_ec_support_24_f00
sym.se_sys_init_ec_qfan_pwm 0x7fd89 [CALL] call sub.se_sys_check_ec_support_24_f00
sym.se_sys_init_ec_tfan_pwm 0x7fdfe [CALL] call sub.se_sys_check_ec_support_24_f00
sym.se_sys_init_ec_tfan_parameter 0x7fe6f [CALL] call sub.se_sys_check_ec_support_24_f00
sym.se_sys_init_ec_qfan_auto_temp 0x7fed9 [CALL] call sub.se_sys_check_ec_support_24_f00
sym.se_sys_set_ec_qfan_auto 0x7ff3c [CALL] call sub.se_sys_check_ec_support_24_f00
sym.se_sys_set_ec_qfan_custom 0x7ff94 [CALL] call sub.se_sys_check_ec_support_24_f00
sym.se_sys_set_ec_qfan_manual 0x7ffec [CALL] call sub.se_sys_check_ec_support_24_f00
sym.se_sys_init_ec_7level_pwm 0x80039 [CALL] call sub.se_sys_check_ec_support_24_f00
sym.se_sys_init_ec_7level_temp 0x8009e [CALL] call sub.se_sys_check_ec_support_24_f00
sym.se_sys_set_ec_7level_auto 0x80119 [CALL] call sub.se_sys_check_ec_support_24_f00
sym.se_sys_set_ec_7level_custom 0x8017e [CALL] call sub.se_sys_check_ec_support_24_f00
sym.se_sys_set_ec_7level_manual 0x801ec [CALL] call sub.se_sys_check_ec_support_24_f00
sym.se_sys_set_ec_tfan_auto 0x8023c [CALL] call sub.se_sys_check_ec_support_24_f00
sym.se_sys_set_ec_tfan_custom 0x80299 [CALL] call sub.se_sys_check_ec_support_24_f00
sym.se_sys_set_ec_tfan_manual 0x802fc [CALL] call sub.se_sys_check_ec_support_24_f00
sym.se_sys_set_ec_tfan_parameters 0x80359 [CALL] call sub.se_sys_check_ec_support_24_f00
sym.se_sys_check_pcie_with_tbt_hba 0x845ad [CALL] call sub.se_sys_check_ec_support_24_f00
sym.se_sys_get_cpld_version 0x86775 [CALL] call sub.se_sys_check_ec_support_24_f00
```

The function `sym.se_sys_set_ec_tfan_manual` seems to be a good candidate. It
calls `se_sys_check_ec_support` then `ec_sys_set_tfan_manual`.

`sym.ec_sys_set_tfan_manual` calls `ioperm()` with values (ports) assigned to
LDN#12. According to radare, it takes a single integer argument. The
`ERR_TRACE()` function will log errors to `/var/log/hal_lib.log.conf`.
if arg2 == 0
 ec_set_single_byte(2, 0x20, 0)
elif arg2 == 1
 ec_set_single_byte(2, 0x23, 0)

This function is called from `sym.ec_sys_set_tfan_manual()`
 

`sym.ec_sys_init_tfan_parameter(arg1, arg2)` also calls `ioperm()` with value
(addresses) assigned to LDN#12. then calls acquire_sio_lock(),
`ec_set_single_byte(2, 0xA8, arg2)` and release_sio_lock().

Here the ec_set_single_byte(arg1(:function), arg2(:index), arg3(:data)) call does the following:
ERR_TRACE(RDI=0x20,
          RSI=0x000faf90 ("%s: func=0x%x, index=0x%x, data=0x%x\n")
          RDX=0x000fc360 ("ec_set_single_byte"),
          RCX=dil (== arg1),
          R8=r13d (== arg2)
          R9=r12d (== arg3)
         )
r13d = sil # arg2 - index
r12d = dl # arg3 - data
ecx = dil # arg1
ebp = edi # arg1 - function
ret = call_in_and_fix_io_addr(0x6C, 0x88) # read
; test something and got to an error handling block
ret = call_in_and_fix_io_addr(0x68, ebp (== arg1) or 0xffffff80) # select function
ret = call_in_and_fix_io_addr(0x68, r13d (== arg2)) # write index
; test something and got to an error handling block
ret = call_in_and_fix_io_addr(0x68, r12d (== arg3)) # write data
; test something and got to an error handling block


libuLinux_hal.so
```
af acquire_sio_lock 0x000b4370
af release_sio_lock 0x000b3be0
af ec_set_single_byte 0x000b3af0
af call_in_and_fix_io_addr 0x000b3a40
```

`ec_sys_set_fan_speed(int fan_region, int speed)` uses these functions and it
able to set the fan speed.

In `hal_event()`, the function `HM_Set_EC_TFan_Manual_370()` seems to dispatch
to different function dealing with fans (TFAN, QFAN, 7Level). arg2 (in RSI) does
the dispatch, 2 is TFAN.

Functions names `HM_*` seems to look for data in the /tmp/em/em_0.info file.

`ec_sys_get_temperature()` uses the `cvtsi2sd` instruction that converts a
signed doubleword integer to a double-precisions floating point. It signature is
likely `ec_sys_get_temperature(int, double*)`.

## Shutdown & Reboot

See https://access.redhat.com/documentation/en-US/Red_Hat_Enterprise_Linux/4/html/Reference_Guide/s3-proc-sys-kernel.html for commands details.

From /etc/init.d/force_shutdown.sh

Example for shutdown:
```
echo s > /proc/sysrq-trigger 
sleep 3
echo u > /proc/sysrq-trigger 
sleep 3
echo o > /proc/sysrq-trigger   
```

Check /etc/acpi/events/

## TODO

ec_sys_get_reset_button
ec_sys_force_shutdown

disk temperature


## Experiments

Calling HM_Set_Fan_Speed(0, 0, 0) stops the fan !

## Methodology

Go to the QNAP control-panel and set fan speed manually.
Go to developper tools, in the network section, click "clear", then "record
network log", click "apply" in the QNAP control panel. Look for new names, then
click "stop" (the "record" button).

Click "preview" and investigate data sent to the server. sysRequest.cgi is the
key. Click on the XML document and look for fan related data.

Retrieve the file with `scp`:
```
[~] # find / -type f |grep sysRequest.cgi
/home/httpd/cgi-bin/sys/sysRequest.cgi
```

```
r2 bin/sysRequest.cgi 
[0x0040e4c0]> izz | grep -i fan
[..]
5128 0x0005c428 0x0045c428  75  76 (.rodata) ascii /sbin/hal_event --se_set_tfan_mode_smart_intelligent fan_region=%d,tmode=%d
5129 0x0005c478 0x0045c478  65  66 (.rodata) ascii /sbin/hal_event --se_set_fan_mode_smart_intelligent fan_region=%d
5130 0x0005c4c0 0x0045c4c0 106 107 (.rodata) ascii /sbin/hal_event --se_set_fan_mode_smart_custom unit=%d,stop_temp=%d,low_temp=%d,high_temp=%d,fan_region=%d
5131 0x0005c530 0x0045c530  65  66 (.rodata) ascii /sbin/hal_event --se_set_fan_mode_fixed fan_level=1 fan_region=%d
5132 0x0005c578 0x0045c578  65  66 (.rodata) ascii /sbin/hal_event --se_set_fan_mode_fixed fan_level=4 fan_region=%d
5133 0x0005c5c0 0x0045c5c0  65  66 (.rodata) ascii /sbin/hal_event --se_set_fan_mode_fixed fan_level=7 fan_region=%d
5134 0x0005c608 0x0045c608  70  71 (.rodata) ascii /sbin/hal_event --se_set_fan_scrollbar_stepping fan_region=%d,value=%d
```

The last line gives the command that can be used to control the fan speed
manually:
```
/sbin/hal_event --se_set_fan_scrollbar_stepping fan_region=0,value=100
```

Download Entware-ng_0.97.qpkg and install it using the web UI. gcc can now be
installed with `/opt/bin/opkg install gcc`

## References

https://jorgbosman.nl/QNAP_TS-459_Pro_with_Ubuntu
https://www.techpowerup.com/reviews/QNAP/TS453B/4.html
  http://www.recomb-omsk.ru/published/SC/html/scripts/doc/94689_datasheet_IT8512E_F_V0.4.1.pdf
https://github.com/torvalds/linux/blob/master/drivers/hwmon/it87.c

https://wiki.qnap.com/wiki/Firmware_Recovery#Firmware_Recovery_Guide_for_TS-x53B_Series.2C_TS-x53BU_Series.2C_TS-x51A_Series.2C_TS-x53A_Series.2C_TBS-453A.2C_TVS-x63_Series.2C_TVS-x63.2B_Series.2C_TVS-x71_Series.2C_TVS-x71U_Series.2C_TVS-x73_Series.2C_TS-x73U_Series.2C_TS-x77_Series.2C_TS-1685.2C_TVS-ECx80_.2F_TVS-ECx80.2B.2C_TVS-ECx80U.C2.A0Series.2C_TVS-x82_Series.2C_TVS-x82T_Series.2C_TVS-x82ST_Series.2C_TVS-882BR.2C_TVS-882BRT3.2C_TVS-1282T3.2C.C2.A0TDS-16489U_NAS

http://eu1.qnap.com/Storage/tsd/fullimage/F_TS-X53B_20161212-1.3.0_EFI.img

https://www.qnap.com/en/product/ts-453be
