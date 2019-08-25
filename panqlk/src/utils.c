// Copyright (C) 2019 Guillaume Valadon <guillaume@valadon.net>

// panql - utils


#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/io.h>

#include <cap-ng.h>

#include "it8528_commands.h"


void ensure_io_capability(void) {
    // Exits panql if privileged I/O port operations are not permitted

    int has_capability = capng_have_capability(CAPNG_EFFECTIVE, CAP_SYS_RAWIO);
    bool is_root = (getuid() == 0 && getuid() == 0);

    if (!has_capability && !is_root) {
        fprintf(stderr, "panql must have the CAP_SYS_RAWIO capability, %s",
                        "or be launched as root!\n");
        exit(EXIT_FAILURE);
    }
}


u_int8_t sio_read(u_int8_t reg) {
    // Read a value from a IT8528 register

    outb(reg, 0x2E);
    return inb(0x2F);
}


bool ensure_it8528(void) {
    // Check if the Super I/O component is an IT8528

    ensure_io_capability();

    if (iopl(3) != 0) {
        fprintf(stderr, "iopl() - %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Access Super I/O configuration registers
    u_int8_t chipid1 = sio_read(0x20);
    u_int8_t chipid2 = sio_read(0x21);
    if (chipid1 == 0x85 && chipid2 == 0x28) {
        return true;
    }
    else {
        fprintf(stderr, "IT8528 not found!\n");
        return false;
    }
}


void verify_get_fan_pwm() {
    // Verify the behavior of the reimplemented function

    int pwm_value_lk = 0xFF;
    int pwm_ret = ec_sys_get_fan_pwm(0, &pwm_value_lk);
    if (pwm_ret != 0) {
        fprintf(stderr, "ec_sys_get_fan_pwm: Incorrect pwm value!\n");
    }

    int pwm_value_re = 0xFF;
    pwm_ret = it8528_get_fan_pwm(0, &pwm_value_re);
    if (pwm_ret != 0) {
        fprintf(stderr, "it8528_get_fan_pwm: Incorrect pwm value!\n");
    }

    if (pwm_value_lk != pwm_value_re) {
        fprintf(stderr,
	        "verify_pwm_value: Incorrect values (%d != %d)!\n",
		pwm_value_lk, pwm_value_re
	       );
    }

}
