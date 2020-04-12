# panq - IT8528 interactions

The `panq` (QNAP reversed) binary provides commands to interact with the ITE IT8528 Super I/O Controller on a [QNAP TS-453Be NAS](https://www.qnap.com/en/product/ts-453be).  Its main goals are to read the chassis temperature and to control the fan speed.  
The [lib/libuLinux_hal.so](libuLinux_hal.so) was first reversed with [radare2](https://github.com/radare/radare2), [Ghidra](https://github.com/NationalSecurityAgency/ghidra), and [QBDI](https://qbdi.quarkslab.com/), then the [necessary functions](src/it8528.c) were reimplemented.


## Implemented Commands

The reimplemented functions from `libuLinux_hal.so` provides the following `panq` features:
```
$ panq
Usage: panq { COMMAND | help }

    Control the I8528 Super I/O controller on QNAP TS-453Be

Available commands:
  check                      - detect the Super I/O controller
  fan [ SPEED_PERCENTAGE ]   - get or set the fan speed
  help                       - this help message
  led { on | off | blink }   - configure the front USB LED
  log                        - display fan speed & temperature
  test [libuLinux_hal.so]    - test functions against libuLinux_hal.so                                                        
  temperature                - retrieve the temperature
```


## Notes

- the binary needs `libcap-ng` and `libseccomp2` to be built.
- to run `panq` as as regular user, use `make capability` 
- according to noise analysis from [techpowerup](https://www.techpowerup.com/review/qnap-ts453b/20.html), the fan will output 22 dBA at 10% speed, 34 dBA at 50% and 44 dBA at 100%


# More Functionalities

The original `libuLinux_hal.so` library contains interesting `ec_sys_*` functions that seems to indicate that the following functionalities could be implemented in `panq`:
- control more LEDs
- enclosure opening detection
- read more temperature sensors
- automatic fan speed (`ec_sys_set_tfan_auto()`, `ec_sys_set_qfan_auto()`...)
