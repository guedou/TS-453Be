// Copyright (C) 2019 Guillaume Valadon <guillaume@valadon.net>

// panql - utils


#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/io.h>

#include <cap-ng.h>


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
