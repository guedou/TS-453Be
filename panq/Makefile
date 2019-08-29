# Copyright (C) 2019 Guillaume Valadon <guillaume@valadon.net>

all: panq

CFLAGS=-Iinclude/
LD_FLAGS=-Llib/ -lcap-ng -ldl

panq: src/main.c src/commands.c src/utils.c src/it8528.c src/it8528_utils.c
	$(CC) -o $@ $^ $(LD_FLAGS) $(CFLAGS)
