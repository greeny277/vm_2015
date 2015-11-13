# $Id: Makefile 10012 2014-10-15 16:25:53Z raimue $

CFLAGS=\
	-g \
	-O2 \
	\
	-Wall \
#	-Werror \

CINCLUDE_FLAGS=\
	-I bus \
	-I comp
CC=gcc
LD=ld -m elf_i386
AS=as -32
OBJCOPY=objcopy

BUS_SRC=\
	bus/sig_host_bus.c

COMP_SRC=\
	comp/disk_ctrl.c \
	comp/serial_ctrl.c \
	comp/cpu.c \
	comp/bios_rom.c \
	comp/memory.c

ROMS_SRC=\
	comp/roms/bios_boot.S\
	comp/roms/bios_print_simple.S \
	comp/roms/bios_print.S

BUS_OBJ=$(patsubst %.c,%.o,$(BUS_SRC))
COMP_OBJ=$(patsubst %.c,%.o,$(COMP_SRC))
ROMS_OBJ=$(patsubst %.S,%.o,$(ROMS_SRC))
ROMS_ROMS=$(patsubst %.S,%.rom,$(ROMS_SRC))
ROMS_ELFS=$(patsubst %.S,%.elf,$(ROMS_SRC))

DEFAULT_DISK_FILE=hallo.img

all: vm_test $(ROMS_ROMS) $(ROMS_OBJ) $(ROMS_ELFS) disk.bin

vm_test: setup.o $(BUS_OBJ) $(COMP_OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	$(RM) $(BUS_OBJ)
	$(RM) $(COMP_OBJ)
	$(RM) vm_test setup.o
	$(RM) $(ROMS_ROMS)
	$(RM) $(ROMS_OBJ)
	$(RM) $(ROMS_ELFS)
	$(RM) disk.bin
	$(MAKE) -C comp/test $@

%.o:%.c
	$(CC) $(CFLAGS) $(CINCLUDE_FLAGS) -c $< -o $@


%.o:%.S
	$(AS) $< -o $@

%.elf:%.o
	$(LD) -T $(dir $<)/bios.lds $< -o $@

# copy ".text" section to output file
%.rom:%.elf
	$(OBJCOPY) -O binary -j .rom $< $@

comp/test/%:
	$(MAKE) -C comp/test $*

disk.bin: comp/test/$(DEFAULT_DISK_FILE)
	cat $< /dev/zero | dd bs=512 count=10240 of=$@

.PHONY: all clean
