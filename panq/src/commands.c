// Copyright (C) 2019 Guillaume Valadon <guillaume@valadon.net>

// panq - commands implementations


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "it8528.h"
#include "utils.h"

#include <sys/io.h>

void command_check(void) {
    // Implements the check command

    if (ensure_it8528()) {
        printf("IT8528 detected.\n");
        exit(EXIT_SUCCESS);
    }
    else {
        exit(EXIT_FAILURE);
    }
}


void command_fan(u_int32_t *speed) {
    // Implements the fan command

    if (!ensure_it8528()) {
        exit(EXIT_FAILURE);
    }

    u_int16_t max_fan_speed = 1720;

    u_int8_t status_value = 0xFF;
    u_int8_t status_ret = it8528_get_fan_status(0, &status_value);
    if (status_ret != 0 && status_value != 0) {
        fprintf(stderr, "Incorrect fan status!\n");
        exit(EXIT_FAILURE);
    }

    if (speed == NULL) {
        u_int32_t speed_value;
        if(it8528_get_fan_speed(0, &speed_value) != 0) {
            fprintf(stderr, "Can't get fan speed!\n");
            exit(EXIT_FAILURE);
        }

	float percent = (float) speed_value / (max_fan_speed-15) * 100;
	if (percent > 100.0) {
	   percent = 100;
	}
        printf("%d RPM (~%.2f%%)\n", speed_value, percent);
    }
    else {

	if (*speed < 0 || *speed > 100) {
            fprintf(stderr, "Invalid percent!\n");
            exit(EXIT_FAILURE);
	}

	// Note: the formula to convert from fan speed to RPM is approximately:
	//       rpm = 7 * fan_speed - 17

	// Convert from fan speed percentage to fan speed
	float fan_speed = (max_fan_speed * *speed / 100);
	fan_speed += 17;
	fan_speed /= 7;

        if(it8528_set_fan_speed(0, (u_int8_t) fan_speed) != 0) {
            fprintf(stderr, "Can't set fan speed!\n");
            exit(EXIT_FAILURE);
        }
    }
}


void command_log(void) {
    // Print the fan speed and the temperature for logging

    if (!ensure_it8528()) {
        exit(EXIT_FAILURE);
    }

    u_int8_t status_value = 0xFF;
    u_int8_t status_ret = it8528_get_fan_status(0, &status_value);
    if (status_ret != 0 && status_value != 0) {
        fprintf(stderr, "Incorrect fan status!\n");
        exit(EXIT_FAILURE);
    }

    u_int32_t speed_value;
    if(it8528_get_fan_speed(0, &speed_value) != 0) {
        fprintf(stderr, "Can't get fan speed!\n");
        exit(EXIT_FAILURE);
    }

    double temperature_value = 0;
    if (it8528_get_temperature(0, &temperature_value) != 0) {
        fprintf(stderr, "Can't get the temperature!\n");
        exit(EXIT_FAILURE);
    }
    printf("%ld,%d,%.2f\n", time(NULL), speed_value, temperature_value);
}


void command_led(char *mode) {
    ensure_io_capability();

    if (!ensure_it8528()) {
        exit(EXIT_FAILURE);
    }

    int led_mode = 0;
    if (strcmp("off", mode) == 0) {
        led_mode = 0;
    }
    else if (strcmp("blink", mode) == 0) {
        led_mode = 1;
    }
    else if (strcmp("on", mode) == 0) {
        led_mode = 2;
    }
    else {
        fprintf(stderr, "Invalid LED mode!\n");
        exit(EXIT_FAILURE);
    }

    if(it8528_set_front_usb_led(led_mode) != 0) {
        fprintf(stderr, "Can't set the USB LED!\n");
        exit(EXIT_FAILURE);
    }

    for(int fan_id=0; fan_id < 5; fan_id++) {
	u_int8_t ret_value, fan_status = 0;
       	u_int32_t fan_speed = 0;
	ret_value = it8528_get_fan_status(fan_id, &fan_status);
	ret_value = it8528_get_fan_speed(fan_id, &fan_speed);
	printf("fan_id=%d ret_value=%d fan_status=%d fan_speed=%d\n", fan_id, ret_value, fan_status, fan_speed);

    }
}


void command_temperature(void) {
    // Implements the temperature command using functions from QNAP

    if (!ensure_it8528()) {
        exit(EXIT_FAILURE);
    }

    double temperature_value = 0;
    if (it8528_get_temperature(0, &temperature_value) != 0) {
        fprintf(stderr, "Can't get the temperature!\n");
        exit(EXIT_FAILURE);
    }


    // Note: the file /etc/hal_util.conf contains the value ADJUST_SYS_TEMP=-2
    //       that could mean that this reading needs to be corrected.
    printf("%.2f °C\n", temperature_value);
}
