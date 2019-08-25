# Copyright (C) 2019 Guillaume Valadon <guillaume@valadon.net>

all: panqlk

CFLAGS=-Iinclude/
LD_FLAGS=-Llib/ -luLinux_hal -lcap-ng -Wl,-rpath,lib/

panqlk: src/main.c src/commands.c src/utils.c src/it8528_commands.c
	$(CC) -o $@ $^ $(LD_FLAGS) $(CFLAGS)
