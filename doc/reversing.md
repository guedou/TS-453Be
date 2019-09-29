# Reversing QNAP Binaries

The TS-453Be fan is controlled using the `hal_event` binary as [previously discovered](https://github.com/guedou/TS-453Be/blob/master/doc/fan_control.md). In order to control the fan without it, the next step consists in identifying the useful functions calls.


## Exploring hal_event

The `hal_event` binary was retrieved from a live QTS installation. The `hal_util` binary, contained in the [firmware recovery initrd](https://github.com/guedou/TS-453Be/blob/master/doc/qts_firmware_recovery.md) is indeed quite similar. On QTS, the binaries `hal_daemon` and `hal_event` are the same. Their behavior is determined by the filename at runtime.

Here is the hash of the file that will be analyzed:
```
rahash2 -a sha256 -qq hal_util
1da51fde1a6aae36bc3dadc8454f82e0b0917357e0d5897b5fda98bb4d0d0909
```

First, let's look for function names used in `hal_util` that include the words `fan` and `speed` with `radare2`,
- `afl` lists functions
- `~` searches for strings
- `+` makes the search case insensitive
- `&` matches all searched words
- `,` separates searched words

```
r2 -A hal_util
[0x0040a270]> afl~+&fan,speed
0x00408950    1 6            sym.imp.HM_Set_Fan_Speed
0x004092d0    1 6            sym.imp.HM_Set_Fan_Speed_By_PWM
```

The `sym.imp.HM_Set_Fan_Speed()` and `sym.imp.HM_Set_Fan_Speed_By_PWM()` looks both interesting, and are imported from on of the 12 external libraries used by `hal_util`, as shown with the following command:
```
[0x0040a270]> 
il
[Linked libraries]
libjson.so.0
libuLinux_hal.so
libuLinux_ini.so
libuLinux_Storage.so
libuLinux_Util.so.0
libuLinux_config.so.0
libuLinux_NAS.so.0
libuLinux_naslog.so.2
libuLinux_quota.so.0
libuLinux_qha.so.0
libpthread.so.0
libc.so.6

12 libraries
```

Next, let's open the first library `libuLinux_hal.so` (90d60e493ed9dd164f5ff6eb918e7f65fde20965b3f52de52d41c2ccd6c50eca) and look for `HM_Set_Fan_Speed`:
```
r2 -A libuLinux_hal.so                               
[0x0001d0a0]> afl~HM_Set_Fan_Speed
0x0002d470    4 135  -> 132  sym.HM_Set_Fan_Speed_By_PWM
0x0002d3e0    4 135  -> 132  sym.HM_Set_Fan_Speed
```

```
[0x0001d0a0]> s sym.HM_Set_Fan_Speed
[0x0002d3e0]> 
```

Exploring the callgraph with `VV` will show that `HM_Set_Fan_Speed()` calls `se_lookup_sys_id()` then `se_sys_set_fan_speed()`.`se_lookup_sys_id()` is short and does nothing more than opening the file `/etc/hal.conf` retrieved with [hal_dump](https://github.com/guedou/TS-453Be/blob/master/doc/fan_control.md), and retrieving the value associated with `enc_sys_id` in the `Enclosure_0` section. `se_sys_set_fan_speed()` is much more interesting: it is a generic function that handles all fan contollers that could be available on QNAP NAS using the following logic on the TS-453Be:
- call to `0x52e20()` that retrieves the `FAN_UNIT` value from `model.conf`. The value 8 is returned for "EC"
- call to `common_sys_get_sys_fan_pwm()` to retrieve fan levels
- call to `ec_sys_set_fan_speed(fan_id, speed)` to control the fan speed

The following C code can be used to check if `ec_sys_set_fan_speed()` can control the fan speed:
```
cat test_fan_speed.c
#include <stdio.h>
#include <stdlib.h>
#include <sys/io.h>

int main(int argc, char **argv) {

    iopl(3);

    int fan_speed = argc > 1 ? atoi(argv[1]) :  50;

    int ec_sys_set_fan_speed(int, int);
    printf("result=%d\n", ec_sys_set_fan_speed(0, fan_speed));
}
```

On QTS, `gcc` can be added after installing `Entware-ng_0.97.qpkg` from the web UI, and typing `/opt/bin/opkg install gcc` in a shell. The folllowing commands will compile the test file and make the fan run at full speed:

```
# PATH=/opt/bin:$PATH; /opt/bin/gcc -o test test.c -luLinux_hal -Llib/
[admin@NAS22477F ~]# LD_LIBRARY_PATH=/lib ./test 1800
result=0
```

On Ubuntu, the `test` binary can also control the fan speed. This is an important result that shows that the fan can be directly controled without any kernel interaction.


# Reimplementing libuLinux_hal.so functions
