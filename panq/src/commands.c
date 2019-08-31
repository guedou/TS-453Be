// Copyright (C) 2019 Guillaume Valadon <guillaume@valadon.net>

// panq - commands implementations


#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "commands.h"
#include "it8528.h"
#include "utils.h"

//#include <sys/io.h>


void command_check(void) {
    // Implements the check command

    if (ensure_it8528()) {
        printf("IT8528 detected.\n");
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
    int8_t status_ret = it8528_get_fan_status(0, &status_value);
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

        double percent = speed_value / ((double) max_fan_speed - 15) * 100;
        if (percent > 100.0) {
            percent = 100;
        }
        printf("%d RPM (~%.2f%%)\n", speed_value, percent);
    }
    else {

        if (*speed > 100) {
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
    int8_t status_ret = it8528_get_fan_status(0, &status_value);
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

    exit(EXIT_SUCCESS);
}


void command_led(char *mode) {
    // Implements the led command

    ensure_io_capability();

    if (!ensure_it8528()) {
        exit(EXIT_FAILURE);
    }

    u_int8_t led_mode = 0;
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

}


void command_test(char* libuLinux_hal_path) {
    // Test the functions with the native ones from libuLinux_hal.so

    if (!ensure_it8528()) {
        exit(EXIT_FAILURE);
    }

    void *handle;
    char *error;

    handle = dlopen(libuLinux_hal_path, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    // it8528_get_fan_status
    typedef int8_t (*ec_sys_get_fan_status_t)(u_int8_t, u_int8_t*);
    ec_sys_get_fan_status_t ec_sys_get_fan_status = (ec_sys_get_fan_status_t) dlsym(handle, "ec_sys_get_fan_status");
    error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
        exit(EXIT_FAILURE);
    }

    int8_t ret_status_ec, ret_status_it;
    u_int8_t value_status_ec, value_status_it = 0;
    ret_status_ec = ec_sys_get_fan_status(0, &value_status_ec);
    ret_status_it = it8528_get_fan_status(0, &value_status_it);
    if (ret_status_ec != ret_status_it || value_status_ec != value_status_it) {
        fprintf(stderr, "it8528_get_fan_status() test failed!\n");
        fprintf(stderr, "ret   %d != %d\n", ret_status_ec, ret_status_it);
        fprintf(stderr, "value %d != %d\n", value_status_ec, value_status_it);
        exit(EXIT_FAILURE);
    }

    // it8528_get_fan_pwm
    typedef int8_t (*ec_sys_get_fan_pwm_t)(u_int8_t, u_int8_t*);
    ec_sys_get_fan_pwm_t ec_sys_get_fan_pwm  = (ec_sys_get_fan_pwm_t) dlsym(handle, "ec_sys_get_fan_pwm");

    error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
        exit(EXIT_FAILURE);
    }

    int8_t ret_pwm_ec, ret_pwm_it;
    u_int8_t value_pwm_ec, value_pwm_it = 0;
    ret_pwm_ec = ec_sys_get_fan_pwm(0, &value_pwm_ec);
    ret_pwm_it = it8528_get_fan_pwm(0, &value_pwm_it);
    if (ret_pwm_ec != ret_pwm_it || value_pwm_ec != value_pwm_it) {
        fprintf(stderr, "it8528_get_fan_pwm() test failed!\n");
        fprintf(stderr, "ret   %d != %d\n", ret_pwm_ec, ret_pwm_it);
        fprintf(stderr, "value %d != %d\n", value_pwm_ec, value_pwm_it);
        exit(EXIT_FAILURE);
    }

    // it8528_get_fan_speed
    typedef int8_t (*ec_sys_get_fan_speed_t)(u_int8_t, u_int32_t*);
    ec_sys_get_fan_speed_t ec_sys_get_fan_speed = (ec_sys_get_fan_speed_t) dlsym(handle, "ec_sys_get_fan_speed");
    error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
        exit(EXIT_FAILURE);
    }

    int8_t ret_speed_ec, ret_speed_it = 0;
    u_int32_t value_speed_ec, value_speed_it = 0;
    ret_speed_ec = ec_sys_get_fan_speed(0, &value_speed_ec);
    ret_speed_it = it8528_get_fan_speed(0, &value_speed_it);
    if (ret_speed_ec != ret_speed_it || value_speed_ec != value_speed_it) {
        fprintf(stderr, "it8528_get_fan_speed() test failed!\n");
        fprintf(stderr, "ret   %d != %d\n", ret_speed_ec, ret_speed_it);
        fprintf(stderr, "value %d != %d\n", value_speed_ec, value_speed_it);
        exit(EXIT_FAILURE);
    }

    // it8528_get_temperature
    typedef int8_t (*ec_sys_get_temperature_t)(u_int8_t, double*);
    ec_sys_get_temperature_t ec_sys_get_temperature = (ec_sys_get_temperature_t) dlsym(handle, "ec_sys_get_temperature");
    error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
        exit(EXIT_FAILURE);
    }

    int8_t ret_temperature_ec, ret_temperature_it = 0;
    double value_temperature_ec, value_temperature_it = 0;
    ret_temperature_ec = ec_sys_get_temperature(0, &value_temperature_ec);
    ret_temperature_it = it8528_get_temperature(0, &value_temperature_it);
    if (ret_temperature_ec != ret_temperature_it || value_temperature_ec < value_temperature_it) {
        fprintf(stderr, "it8528_get_temperature() test failed!\n");
        fprintf(stderr, "ret   %d != %d\n", ret_temperature_ec, ret_temperature_it);
        fprintf(stderr, "value %f != %f\n", value_temperature_ec, value_temperature_it);
        exit(EXIT_FAILURE);
    }

    printf("All tests passed.\n");
    dlclose(handle);
}


void command_temperature(void) {
    // Implements the temperature command

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
