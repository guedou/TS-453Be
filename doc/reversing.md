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

The `sym.imp.HM_Set_Fan_Speed()` and `sym.imp.HM_Set_Fan_Speed_By_PWM()` looks both interesting, and are imported from one of the 12 external libraries used by `hal_util`, as shown with the following command:
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

Exploring the callgraph with `VV` will show that `HM_Set_Fan_Speed()` calls `se_lookup_sys_id()` then `se_sys_set_fan_speed()`.`se_lookup_sys_id()` is short and does nothing more than opening the file `/etc/hal.conf` retrieved with [hal_dump](https://github.com/guedou/TS-453Be/blob/master/doc/fan_control.md), and retrieving the value associated with `enc_sys_id` in the `Enclosure_0` section. `se_sys_set_fan_speed()` is much more interesting: it is a generic function that handles all fan controllers that could be available on QNAP NAS using the following logic on the TS-453Be:
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

On QTS, `gcc` can be added after installing `Entware-ng_0.97.qpkg` from the web UI, and typing `/opt/bin/opkg install gcc` in a shell. The following commands will compile the test file and make the fan run at full speed:

```
# PATH=/opt/bin:$PATH; /opt/bin/gcc -o test test.c -luLinux_hal -Llib/
[admin@NAS22477F ~]# LD_LIBRARY_PATH=/lib ./test 1800
result=0
```

On Ubuntu, the `test` binary can also control the fan speed. This is an important result that shows that the fan can be directly controlled without any kernel interaction.


## Reimplementing libuLinux_hal.so Functions

Most of the functions reimplemented in [panq](https://github.com/guedou/TS-453Be/blob/master/panq/) were statically analyzed wi
with radare2. When needed, dynamic instrumentation with [QBDI](https://qbdi.quarkslab.com/) and [Ghidra decompiler](https://github.com/radareorg/r2ghidra-dec) were also used.

### Tracing with QBDI

Interacting with the IT8528 EC is done with the `inb()` and `outb()` macros that correspond to the `in` and `out` x86 instructions. QBDI can help tracing these instructions and understanding the expected behavior.

Creating a Docker image that includes QBDI, here guedou/qbdi, can easily be done with the following commands:
```
docker pull ubuntu:18.10
docker run --name qbdi guedou/ubuntu
docker exec -it qbdi bash
apt update ; apt install wget vim python-pip npm cmake
# install qbdi & frida as described in https://qbdi.readthedocs.io/en/stable/
# exit the docker image
docker commit qbdi guedou/qbdi
```

In order to display the relevant instructions, just create the following Python script based on the examples available on the QBDI website:
```
import pyqbdi

def mycb(vm, gpr, fpr, data):
    inst = vm.getInstAnalysis()
    state = vm.getGPRState()
    if "in" in inst.disassembly:
        print "0x%x: %s # 0x%x, 0x%x" % (inst.address, inst.disassembly, state.rax, state.rdx)
    elif "out" in inst.disassembly:
        print "0x%x: %s # 0x%x, 0x%x" % (inst.address, inst.disassembly, state.rdx, state.rax)
    return pyqbdi.CONTINUE

def pyqbdipreload_on_run(vm, start, stop):
    vm.addCodeCB(pyqbdi.PREINST, mycb, None)
    vm.run(start, stop)
```

Launching the script is simple, first start the Docker image with the SYS_RAWIO capability, then launch the script on the studied binary:
```
docker run --rm -it -v $PWD:/qnap --cap-add=SYS_RAWIO guedou/qbdi  bash
root@29e120d31d3e:/# LD_PRELOAD=/usr/lib/libpyqbdi.so PYQBDI_TOOL=/qnap/pyqbdi_trace.py /qnap/panq temperature |egrep 'in|out'
0x55ce91e90bba:         out     dx, al # 0x2e, 0x20
0x55ce91e90b99:         in      al, dx # 0x2f, 0x2f
0x55ce91e90bba:         out     dx, al # 0x2e, 0x21
0x55ce91e90b99:         in      al, dx # 0x2f, 0x2f
0x55ce91e8f4f7:         in      al, dx # 0x6c, 0x6c
0x55ce91e8f4f7:         in      al, dx # 0x6c, 0x6c
0x55ce91e8f4f7:         in      al, dx # 0x68, 0x68
0x55ce91e8f4f7:         in      al, dx # 0x6c, 0x6c
0x55ce91e8f518:         out     dx, al # 0x6c, 0x88
0x55ce91e8f4f7:         in      al, dx # 0x6c, 0x6c
0x55ce91e8f518:         out     dx, al # 0x68, 0x6
0x55ce91e8f4f7:         in      al, dx # 0x6c, 0x6c
0x55ce91e8f518:         out     dx, al # 0x68, 0x0
0x55ce91e8f4f7:         in      al, dx # 0x6c, 0x6c
0x55ce91e8f4f7:         in      al, dx # 0x6c, 0x6c
0x55ce91e8f4f7:         in      al, dx # 0x68, 0x68
```

### Ghidra Decompiler For radare2

Since September 2019, the Ghidra decompiler can easily be accessed as a radare2 plugin. On Ubuntu, the following commands will install it:
```
apt install cmake libbison-dev libfl-dev
r2pm -i r2ghidra-dec
```

Understanding functions behavior is now much simpler, here is an example with `ec_sys_set_fan_speed()` after renaming some functions:
```
r2 -A libuLinux_hal.so                               
[0x0001d0a0]> s sym.ec_sys_set_fan_speed
[0x000b5590]> pdg

// WARNING: [r2ghidra] Failed to find return address in ProtoModel                                                            

uint64_t sym.ec_sys_set_fan_speed(int32_t arg1, int32_t arg2)
{                                                                                                                             
    int32_t iVar1;
    uint32_t uVar2;
    undefined8 uVar3;
    undefined4 uStack44;

    uStack44 = 0;
    sym.imp.ioperm(0x6c, 1, 1);
    sym.imp.ioperm(0x68, 1, 1);
    if ((arg1 < 0) || ((4 < arg1 && (1 < arg1 - 6U)))) {
        uVar3 = 0x703;
    } else {
        ERR_TRACE(0x10, "%s(%d): Set fan%d pwm = %d(%d%)\n", "ec_sys_set_fan_speed", 0x707, (uint64_t)(uint32_t)arg1,         
                  (uint64_t)(uint32_t)arg2);
        loc.imp.Ini_Conf_Get_Field_Int("/etc/model.conf", "System Enclosure", "MAX_CPU_FAN_NUM", &uStack44, 0);               
        iVar1 = acquire_sio_lock((int32_t)register0x00000020 + -0x30);                                                        
        if (-1 < iVar1) {
            iVar1 = ec_set_single_byte(2);
            if (iVar1 != 0) {
                ERR_TRACE(1, "%s(%d): System Error\n", "ec_sys_set_fan_speed", 0x712);                                        
            }
            uVar2 = ec_set_single_byte(2);
            if (uVar2 != 0) {
                ERR_TRACE(1, "%s(%d): System Error\n", "ec_sys_set_fan_speed", 0x715);                                        
            }
            release_sio_lock(0);
            return (uint64_t)uVar2;
        }
        uVar3 = 0x70d;
    }
    ERR_TRACE(1, "%s(%d): System Error\n", "ec_sys_set_fan_speed", uVar3);                                                    
    return 0xffffffff;
}
```
