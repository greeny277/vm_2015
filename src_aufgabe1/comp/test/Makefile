# $Id: Makefile 10011 2014-10-15 16:08:01Z raimue $

AS=as -32
LD=ld -m elf_i386
OBJCOPY=objcopy

LIBS_LBF_OBJ=vm_libs.o
LIBS_LINUX_OBJ=linux_libs.o

ASM_SOURCES=\
	hallo.S \
	arith.S
ASM_IMGS=$(patsubst %.S,%.img,$(ASM_SOURCES))
ASM_BINS=$(patsubst %.S,%.bin,$(ASM_SOURCES))
ASM_ELFS=$(patsubst %.S,%.elf,$(ASM_SOURCES))

all: \
	$(ASM_IMGS) \
	$(ASM_BINS) \
	$(ASM_ELFS)

clean:
	$(RM) *.o
	$(RM) $(ASM_IMGS)
	$(RM) $(ASM_BINS)
	$(RM) $(ASM_ELFS)

%.o:%.S
	$(AS) $< -o $@

# from non bios sources, generate a linux binary with using the linux libs
# implementation
# no linker script is required here
%.bin:%.o $(LIBS_LINUX_OBJ)
	$(LD) -o $@ $^

%.elf:%.o $(LIBS_LBF_OBJ)
	$(LD) -T $(dir $<)/disk.lds -o $@ $^

%.img:%.elf
	$(OBJCOPY) -O binary -j .disk $< $@

.PHONY: all clean
