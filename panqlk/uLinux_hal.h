// Copyright (C) 2019 Guillaume Valadon <guillaume@valadon.net>

// libuLinux_hal.so functions prototypes


int ec_sys_get_fan_speed(int, int*);
int ec_sys_set_fan_speed(int, u_int8_t);
int ec_sys_get_fan_status(int, int*);

int ec_sys_set_front_usb_led(int);

int ec_sys_get_temperature(int, double*);


// Fake functions from libuLinux_ini.so to make the linker happy

#define MAKE_FAKE_FUNCTION(name) void name(void) {}
MAKE_FAKE_FUNCTION(Ini_Conf_Get_Field)
MAKE_FAKE_FUNCTION(Ini_Conf_Bitmap_Enumerate)
MAKE_FAKE_FUNCTION(Ini_Conf_Bitmap_Is_Bit_Set)
MAKE_FAKE_FUNCTION(Ini_Conf_Unlock_File)
MAKE_FAKE_FUNCTION(Ini_Conf_Bitmap_Get_Next_Free_Bit)
MAKE_FAKE_FUNCTION(Ini_Conf_Write_Lock_File)
MAKE_FAKE_FUNCTION(Ini_Conf_Get_Field_Int)
MAKE_FAKE_FUNCTION(Ini_Conf_Set_Field)
MAKE_FAKE_FUNCTION(Ini_Conf_Remove_Section)
MAKE_FAKE_FUNCTION(Ini_Conf_Set_Field_Int)
MAKE_FAKE_FUNCTION(Ini_Conf_Remove_Field)
MAKE_FAKE_FUNCTION(Ini_Conf_Bitmap_Set_Bit)
MAKE_FAKE_FUNCTION(Ini_Conf_Bitmap_Get_Set_Count)
MAKE_FAKE_FUNCTION(Ini_Conf_Bitmap_Reset_Bit)
