// Copyright (C) 2019 Guillaume Valadon <guillaume@valadon.net>

// panq - IT8528 utils prototypes


int8_t it8528_check_ready(u_int8_t, u_int8_t);
int8_t it8528_send_commands(u_int8_t, u_int8_t);
int8_t it8528_get_double(u_int8_t, u_int8_t, double*);
int8_t it8528_get_byte(u_int8_t, u_int8_t, u_int8_t*);
int8_t it8528_set_byte(u_int8_t, u_int8_t, u_int8_t);
