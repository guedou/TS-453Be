# Copyright (C) 2019 Guillaume Valadon <guillaume@valadon.net>

CFLAGS=-Werror -Iinclude/
LD_FLAGS=-Llib/ -lcap-ng -ldl

# Ignore errors
.IGNORE: clean

# Project management targets
all: panq

panq: src/*.c
	$(CC) -o $@ $^ $(LD_FLAGS) $(CFLAGS)

capability: panq
	setcap cap_sys_rawio+ep panq

clean:
	@rm panq
