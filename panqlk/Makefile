# Copyright (C) 2019 Guillaume Valadon <guillaume@valadon.net>

FW_URL=http://eu1.qnap.com/Storage/tsd/fullimage/F_TS-X53B_20161212-1.3.0_EFI.img

F_TS-X53B_20161212-1.3.0_EFI.img:
	curl -O $(FW_URL)

#all: F_TS-X53B_20161212-1.3.0_EFI.img
#	echo $(FW_URL)

all: panqlk

LD_FLAGS=-L./ -luLinux_hal -Wl,-rpath,./ -lcap-ng

panqlk: main.c
	$(CC) -o $@ $^ $(LD_FLAGS)
