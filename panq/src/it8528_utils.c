// Copyright (C) 2019 Guillaume Valadon <guillaume@valadon.net>

// panq - IT8528 utils



#include <stdlib.h>
#include <sys/io.h>
#include <unistd.h>


#define IT8528_CHECK_RETRIES 400
#define IT8528_INPUT_BUFFER_FULL 2
#define IT8528_OUTPUT_BUFFER_FULL 1


u_int8_t it8528_check_ready(u_int8_t port, u_int8_t bit_value) {
    // Ensure that the port is ready to be used
    
    int retries = IT8528_CHECK_RETRIES;
    int value = 0;

    do { 
	value = inb(port);
	usleep(250);

	if ((value & bit_value) == 0) {
	    return 0;
	}
    }
    while (retries--);

    return -1;
}


u_int8_t it8528_send_commands(u_int8_t command0, u_int8_t command1) {

    u_int8_t ret_value;

    ret_value = it8528_check_ready(0x6C, IT8528_OUTPUT_BUFFER_FULL);
    if (ret_value != 0) {
	return ret_value;
    }
    inb(0x68);

    ret_value = it8528_check_ready(0x6C, IT8528_INPUT_BUFFER_FULL);
    if (ret_value != 0) {
	return ret_value;
    }
    outb(0x88, 0x6C);

    ret_value = it8528_check_ready(0x6C, IT8528_INPUT_BUFFER_FULL);
    if (ret_value != 0) {
	return ret_value;
    }
    outb(command0, 0x68);

    ret_value = it8528_check_ready(0x6C, IT8528_INPUT_BUFFER_FULL);
    if (ret_value != 0) {
	return ret_value;
    }
    outb(command1, 0x68);

    ret_value = it8528_check_ready(0x6C, IT8528_INPUT_BUFFER_FULL);
    if (ret_value != 0) {
	return ret_value;
    }

    return 0;
 }


u_int8_t it8528_get_double(u_int8_t command0, u_int8_t command1, double *value) {
    u_int8_t ret_value;

    ret_value = it8528_send_commands(command0, command1);
    if (ret_value != 0) {
	return ret_value;
    }
    *value = inb(0x68);
     
    return 0;
}


u_int8_t it8528_get_byte(u_int8_t command0, u_int8_t command1, u_int8_t *value) {
    u_int8_t ret_value;

    ret_value = it8528_send_commands(command0, command1);
    if (ret_value != 0) {
	return ret_value;
    }
    *value = inb(0x68);
     
    return 0;
}


u_int8_t it8528_set_byte(u_int8_t command0, u_int8_t command1, u_int8_t value) {
    u_int8_t ret_value;

    ret_value = it8528_check_ready(0x6C, IT8528_INPUT_BUFFER_FULL);
    if (ret_value != 0) {
	return ret_value;
    }
    outb(0x88, 0x6C);

    ret_value = it8528_check_ready(0x6C, IT8528_INPUT_BUFFER_FULL);
    if (ret_value != 0) {
	return ret_value;
    }
    outb(command0 | 0x80, 0x68);

    ret_value = it8528_check_ready(0x6C, IT8528_INPUT_BUFFER_FULL);
    if (ret_value != 0) {
	return ret_value;
    }
    outb(command1, 0x68);

    ret_value = it8528_check_ready(0x6C, IT8528_INPUT_BUFFER_FULL);
    if (ret_value != 0) {
	return ret_value;
    }
    outb(value, 0x68);
     
    return 0;
}
