// Copyright (C) 2019 Guillaume Valadon <guillaume@valadon.net>

// panq - utils


#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/io.h>
#include <unistd.h>

#include <cap-ng.h>
#include <seccomp.h>

#include "utils.h"


scmp_filter_ctx configure_seccomp(void) {
    // Main seccomp rules

    // Configure seccomp filters
    scmp_filter_ctx scmp_ctx = NULL;

    scmp_ctx = seccomp_init(SCMP_ACT_KILL_PROCESS);
    if (scmp_ctx == NULL)
    {
        fprintf(stderr, "Can't initialize seccomp!\n");
        exit(EXIT_FAILURE);
    }

    int scmp_rules_status = 0;

    // Accept any arguments
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(capget), 0);
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(close), 0);
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0);
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(fstat), 0);
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(gettid), 0);
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(getuid), 0);
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(geteuid), 0);
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(nanosleep), 0);
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(clock_nanosleep), 0);
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 0);

    // Restrict syscall arguments values
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(iopl), 1,
                                          SCMP_A0(SCMP_CMP_EQ, 3));
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(ioperm), 1,
                                          SCMP_A0(SCMP_CMP_EQ, 0x6C));
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(ioperm), 1,
                                          SCMP_A0(SCMP_CMP_EQ, 0x68));
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(openat), 1,
                                          SCMP_A2(SCMP_CMP_MASKED_EQ, O_RDONLY, O_RDONLY));
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 1,
                                          SCMP_A0(SCMP_CMP_EQ, STDOUT_FILENO));
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 1,
                                          SCMP_A0(SCMP_CMP_EQ, STDERR_FILENO));

    if (scmp_rules_status != 0)
    {
        fprintf(stderr, "Can't add seccomp rules!\n");
        exit(EXIT_FAILURE);
    }

    return scmp_ctx;
}


scmp_filter_ctx update_seccomp(scmp_filter_ctx scmp_ctx) {
    // Test sub-command seccomp rules

    int scmp_rules_status = 0;

    // Accept any arguments
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(getcwd), 0);
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(lseek), 0);
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(mmap), 0);
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(mprotect), 0);
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(munmap), 0);
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(prlimit64), 0);
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(rt_sigaction), 0);
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(rt_sigprocmask), 0);
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(semctl), 0);
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(semget), 0);
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(semop), 0);
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(set_robust_list), 0);
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(set_tid_address), 0);
    scmp_rules_status += seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(stat), 0);

    if (scmp_rules_status != 0)
    {
        fprintf(stderr, "Can't add seccomp rules!\n");
        exit(EXIT_FAILURE);
    }

    return scmp_ctx;
}


void ensure_io_capability(void) {
    // Exits panq if privileged I/O port operations are not permitted

    int has_capability = capng_have_capability(CAPNG_EFFECTIVE, CAP_SYS_RAWIO);
    bool is_root = (getuid() == 0 && geteuid() == 0);

    if (!has_capability && !is_root) {
        fprintf(stderr, "panq must have the CAP_SYS_RAWIO capability, %s",
                        "or be launched as root!\n");
        exit(EXIT_FAILURE);
    }
}


u_int8_t sio_read(u_int8_t reg) {
    // Read a value from a IT8528 register

    outb(reg, 0x2E);  // address port
    return inb(0x2F); // data port
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
