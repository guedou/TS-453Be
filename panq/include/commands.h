// Copyright (C) 2019 Guillaume Valadon <guillaume@valadon.net>

// panq - commands prototypes


void command_check(void);
void command_fan(u_int32_t*);
void command_log(void) __attribute__((noreturn));
void command_led(char*);
void command_test(char*);
void command_temperature(void);

void usage(void) __attribute__((noreturn));
