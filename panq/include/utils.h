// Copyright (C) 2020 Guillaume Valadon <guillaume@valadon.net>

// panq - utils prototypes


#include <stdbool.h>

#include "seccomp.h"


scmp_filter_ctx configure_seccomp(void);
scmp_filter_ctx update_seccomp(scmp_filter_ctx);
void ensure_io_capability(void);
u_int8_t sio_read(u_int8_t);
bool ensure_it8528(void);
void verify_get_fan_pwm(void);
