// Copyright (C) 2019 Guillaume Valadon <guillaume@valadon.net>

// panql - IT8528 commands


#include <stdlib.h>
#include <stdio.h>
#include <sys/io.h>
#include <unistd.h>

#include "it8528_utils.h"


u_int8_t it8528_get_fan_status(u_int8_t fan_id, u_int8_t* status_value) {
    ioperm(0x6c, 1, 1);
    ioperm(0x68, 1, 1);

    u_int8_t ret_value;
    ret_value = it8528_get_byte(2, 0x42, status_value);
    if (ret_value != 0) {
        return ret_value;
    }

    return 0;
}


u_int8_t it8528_get_fan_pwm(u_int8_t fan_id, u_int8_t* pwm_value) {
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

    u_int8_t ret_value;
   
    ret_value = it8528_get_byte(2, command, &tmp_pwm_value);
    if (ret_value != 0) {
        return ret_value;
    }

    *pwm_value = (tmp_pwm_value * 0x100 - tmp_pwm_value) / 100;

    return 0;
}


u_int8_t it8528_get_fan_speed(u_int8_t fan_id, u_int32_t* speed_value) {

    u_int8_t byte0;
    u_int8_t byte1;

    ioperm(0x6c, 1, 1);
    ioperm(0x68, 1, 1);

    switch(fan_id) {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
	byte0 = (fan_id * 2) + 0x24;
	byte1 = (fan_id * 2) + 0x25;
	break;
      case 6:
      case 7:
	byte0 = (fan_id * 2) + 0x14;
	byte1 = (fan_id * 2) + 0x15;
	break;
      case 10:
	byte1 = 0x5a;
	byte0 = 0x5b;
	break;
      case 11:
	byte1 = 0x5d;
	byte0 = 0x5e;
        break;
      default:
	fprintf(stderr, "it8528_get_fan_speed: invalid fan ID!\n");
        return -1;
    }

    u_int8_t ret_value, tmp_value = 0;

    ret_value = it8528_get_byte(6, byte0, &tmp_value);
    if (ret_value != 0) {
        return ret_value;
    }
    *speed_value = tmp_value << 8;

    ret_value = it8528_get_byte(6, byte1, &tmp_value);
    if (ret_value != 0) {
        return ret_value;
    }
    *speed_value += tmp_value;

    return 0;
}


u_int8_t it8528_get_temperature(u_int8_t sensor_id, double* temperature_value) {
    u_int8_t command = 0;

    ioperm(0x6c, 1, 1);
    ioperm(0x68, 1, 1);

    command = sensor_id;
 
    switch(sensor_id) {
      case 0:
      case 1:
          break;
      case 5:
      case 6:
      case 7:
	command -= 3;
	break;
      case 10:
	command = 0x59;
	break;
      case 11:
	command = 0x5C;
	break;
      case 0x0F:
      case 0x10:
      case 0x11:
      case 0x12:
      case 0x13:
      case 0x14:
      case 0x15:
      case 0x16:
      case 0x17:
      case 0x18:
      case 0x19:
      case 0x1A:
      case 0x1B:
      case 0x1C:
      case 0x1D:
      case 0x1E:
      case 0x1F:
      case 0x20:
      case 0x21:
      case 0x22:
      case 0x23:
      case 0x24:
      case 0x25:
      case 0x26:
	command = command - 9;
	break;
      default:
	command = 0xD6; // 0x4D6 in the original binary
	break;
    }

    u_int8_t ret_value;

    ret_value = it8528_get_double(6, command, temperature_value);
    if (ret_value != 0) {
        return ret_value;
    }

    return 0;
}


u_int8_t it8528_set_front_usb_led(u_int8_t led_mode) {

    if (led_mode > 3)  {
       fprintf(stderr, "it8528_set_front_usb_led: invalid LED mode!\n");
       return -1;
    }

    u_int8_t ret_value;

    ret_value = it8528_set_byte(1, 0x54, led_mode);
    if (ret_value != 0) {
        return ret_value;
    }

    return 0;
}


u_int8_t it8528_set_fan_speed(u_int8_t fan_id, u_int8_t fan_speed) {

    u_int8_t command0, command1;

    if (fan_id < 0) {
	fprintf(stderr, "it8528_set_fan_speed: invalid fan ID!\n");
        return -1;
    }
    if (fan_id < 5) {
	command0 = 0x20;
	command1 = 0x2e;
    }
    else {
	command0 = 0x23;
	command1 = 0x4b;
    }

    u_int8_t ret_value;

    ret_value = it8528_set_byte(2, command0, 0x10);
    if (ret_value != 0) {
        return ret_value;
    }

    u_int8_t fan_speed_normalized = (fan_speed * 100) / 0xFF;
    ret_value = it8528_set_byte(2, command1, fan_speed_normalized);
    if (ret_value != 0) {
        return ret_value;
    }

    return 0;
}
