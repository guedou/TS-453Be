// Copyright (C) 2019 Guillaume Valadon <guillaume@valadon.net>

// panql - Interact with the IT8528 with libuLinux_hal.so from QNAP


#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/io.h>
#include <unistd.h>

#include "uLinux_hal.h"


void usage(void) {
    // Print usage

    printf("Usage: panql { COMMAND | help }\n\n");
    printf("    Control the I8528 Super I/O controller on QNAP TS-453Be\n\n");
    printf("Available commands:\n");
    printf("  check                    - detect the Super I/O controller\n");
    printf("  fan [ SPEED ]            - get or set the fan speed\n");
    printf("  help                     - this help message\n");
    printf("  led { on | off | blink } - configure the front USB LED\n");
    printf("  temperature              - retrieve the temperature\n");
    printf("\n");

    exit(EXIT_FAILURE);
}


void ensure_root(void) {
    // Exits panqlk if the current user is not root

    bool is_root = (getuid() == 0 && getuid() == 0);

    if (!is_root) {
        fprintf(stderr, "panqlk must be launched as root!\n");
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

    ensure_root();

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
    // Implements the fan command using functions from QNAP

    if (!ensure_it8528()) {
        exit(EXIT_FAILURE);
    }

    int status_value = 0xFF;
    int status_ret = ec_sys_get_fan_status(0, &status_value);
    if (status_ret != 0 && status_value != 0) {
        fprintf(stderr, "Incorrect fan status!\n");
        exit(EXIT_FAILURE);
    }

    if (speed == NULL) {
        int speed_value;
        if(ec_sys_get_fan_speed(0, &speed_value) != 0) {
            fprintf(stderr, "Can't get fan speed!\n");
            exit(EXIT_FAILURE);
        }
        // TODO: 5% per 5% up to 0 and 100 ?
        printf("%d RPM (~%.2f%%)\n", speed_value, speed_value / (float)1700 * 100);
    }
    else {
        // TODO: y = 7*x - 17
        //       110 is a light sound
        if(ec_sys_set_fan_speed(0, *speed) != 0) {
            fprintf(stderr, "Can't set fan speed!\n");
            exit(EXIT_FAILURE);
        }
    }

}


void command_led(char *mode) {
    ensure_root();
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
        fprintf(stderr, "Invalide LED mode!\n");
        exit(EXIT_FAILURE);
    }

    if(ec_sys_set_front_usb_led(led_mode) != 0) {
        fprintf(stderr, "Can't set the USB LED!\n");
        exit(EXIT_FAILURE);
    }
}


void command_temperature(void) {
    // Implements the temperature command using functions from QNAP

    if (!ensure_it8528()) {
        exit(EXIT_FAILURE);
    }

    double temperature_value = 0;
    if (ec_sys_get_temperature(0, &temperature_value) != 0) {
        fprintf(stderr, "Can't get the temperature!\n");
        exit(EXIT_FAILURE);
    }
    printf("%.2f °C\n", temperature_value); // TODO: there is a 2 degrees difference with this value (see ADJUST_SYS_TEMP=-2 in /etc/hal_util.conf)
}


int main(int argc, char **argv) {

    if (argc < 2) {
        usage();
    }

    char *command = argv[1];
    if (strcmp("help", command) == 0) {
        usage();
    }
    else if (strcmp("check", command) == 0) {
        command_check();
    }
    else if (strcmp("fan", command) == 0) {
        u_int32_t *speed = NULL;
        if (argv[2]) {
            speed = (u_int32_t*) malloc(sizeof(u_int32_t));
            *speed = atoi(argv[2]);
        }
        command_fan(speed);
    }
    else if (strcmp("led", command) == 0) {
        if (argv[2]) {
            command_led(argv[2]);
        }
        else {
            fprintf(stderr, "a LED mode is needed!\n");
            exit(EXIT_FAILURE);
        }
    }
    else if (strcmp("temperature", command) == 0) {
        command_temperature();
    }
    else {
        usage();
    }
   
    exit(EXIT_SUCCESS);
}
