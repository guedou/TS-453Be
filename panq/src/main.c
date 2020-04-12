// Copyright (C) 2020 Guillaume Valadon <guillaume@valadon.net>

// panq - Interact with the IT8528 Embedded Controller


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "commands.h"
#include "utils.h"

#include <seccomp.h>


void usage(void) {
    // Print usage

    printf("Usage: panq { COMMAND | help }\n\n");
    printf("    Control the I8528 Super I/O controller on QNAP TS-453Be\n\n");
    printf("Available commands:\n");
    printf("  check                      - detect the Super I/O controller\n");
    printf("  fan [ SPEED_PERCENTAGE ]   - get or set the fan speed\n");
    printf("  help                       - this help message\n");
    printf("  led { on | off | blink }   - configure the front USB LED\n");
    printf("  log                        - display fan speed & temperature\n");
    printf("  test [libuLinux_hal.so]    - test functions against libuLinux_hal.so\n");
    printf("  temperature                - retrieve the temperature\n");
    printf("\n");

    exit(EXIT_FAILURE);
}


int main(int argc, char **argv) {

    if (argc < 2) {
        usage();
    }

    scmp_filter_ctx scmp_ctx = configure_seccomp();

    // Parse arguments and call sub-commands

    char *command = argv[1];
    if (strcmp("help", command) == 0) {
        usage();
    }
    else if (strcmp("check", command) == 0) {
        seccomp_load(scmp_ctx);
        command_check();
    }
    else if (strcmp("fan", command) == 0) {
        seccomp_load(scmp_ctx);
        u_int32_t *speed = NULL;
        if (argv[2]) {
            speed = (u_int32_t*) malloc(sizeof(u_int32_t));
            *speed = (u_int32_t) strtoul(argv[2], NULL, 10);
        }
        command_fan(speed);
    }
    else if (strcmp("led", command) == 0) {
        seccomp_load(scmp_ctx);
        if (argv[2]) {
            command_led(argv[2]);
        }
        else {
            fprintf(stderr, "a LED mode is needed!\n");
            exit(EXIT_FAILURE);
        }
    }
    else if (strcmp("log", command) == 0) {
        seccomp_load(scmp_ctx);
        command_log();
    }
    else if (strcmp("test", command) == 0) {
        seccomp_load(update_seccomp(scmp_ctx));
        if (argv[2]) {
            command_test(argv[2]);
        }
        else {
            command_test("libuLinux_hal.so");
        }
    }
    else if (strcmp("temperature", command) == 0) {
        seccomp_load(scmp_ctx);
        command_temperature();
    }
    else {
        usage();
    }
   
    exit(EXIT_SUCCESS);
}
