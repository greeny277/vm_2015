# $Id: Makefile 10011 2014-10-15 16:08:01Z raimue $

AS=as -32
LD=ld -m elf_i386
OBJCOPY=objcopy

VM_LIB_OBJ=../lib_vm/vm_libs.o main.o ../lib_vm/setup_serial_irq.o
ASM_SOURCES=test-pic.S

ASM_IMGS=$(patsubst %.S,%.rom,$(ASM_SOURCES))
ASM_ELFS=$(patsubst %.S,%.elf,$(ASM_SOURCES))

all: \
	$(ASM_IMGS) \
	$(ASM_BINS) \
	$(ASM_ELFS)

clean:
	$(RM) *.o
	$(RM) ../lib_vm/*.o
	$(RM) $(ASM_IMGS)
	$(RM) $(ASM_ELFS)

%.o:%.S
	$(AS) $< -o $@

%.elf:%.o $(VM_LIB_OBJ)
	$(LD) -T $(dir $<)/bios.lds -o $@ $^

%.rom:%.elf
	$(OBJCOPY) -O binary -j .rom -j .data -j .rodata $< $@

.PHONY: all clean
