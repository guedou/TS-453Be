// Copyright (C) 2019 Guillaume Valadon <guillaume@valadon.net>

// panql - IT8528 commands


#include <stdlib.h>
#include <stdio.h>
#include <sys/io.h>


int it8528_get_fan_pwm(int fan_id, int* pwm_value) {
    u_int8_t tmp_pwm_value = 0;
    u_int8_t command = 0;

    ioperm(0x6c, 1, 1);
    ioperm(0x68, 1, 1);

    if (fan_id < 0) {
	fprintf(stderr, "it8528_get_fan_pwm: invalid fan ID!\n");
        return -1;
    }
    if (fan_id < 5) {
	command = 0x2e;
    }
    else {
	command = 0x4b;
    }

    // ec_get_single_byte()
    outb(command, 0x6c); // write
    tmp_pwm_value = inb(0x68); // read

    *pwm_value = (tmp_pwm_value * 0x100 - tmp_pwm_value) / 100;

    return 0;
}
