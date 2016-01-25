#
# $Id: Makefile 10011 2014-10-15 16:08:01Z raimue $
#

all: bios_boot.rom disk.img

AS=as -32
LD=ld -m elf_i386

%.o: %.S
	$(AS) -o $*.o $*.S

# FIXME
clean distclean::
	$(RM) *.o
	$(RM) ../lib_real/*.o
	$(RM) *.bin
	$(RM) *.elf
	$(RM) *.rom
	$(RM) *.img

bios_boot.rom: bios_boot.elf
	objcopy -O binary -j .text -j .rodata $^ $@

bios_boot.elf: bios_boot.o
	$(LD) -T bios.lds -o $@ $^

main.elf: \
		main.o \
		../lib_vm/vm_libs.o \
		../lib_vm/kbd_ctrl.o
	$(LD) -T x86_cdrom/process.lds -o $@ $^

# yes, we can indeed use linux_libs now, since main.S provides 
# the used features of int $0x80
hallo.elf: \
		hallo.o \
		../lib_real/linux_libs.o
	$(LD) -T x86_cdrom/process.lds -o $@ $^

arith.elf: 	arith.o \
		../lib_real/linux_libs.o
	$(LD) -T x86_cdrom/process.lds -o $@ $^

gp.elf: gp.o
	$(LD) -T x86_cdrom/process.lds -o $@ $^

disk.img: \
		main.elf \
		hallo.elf \
		arith.elf \
		gp.elf \
		../lib_vm/combine_rom
	objcopy -O binary -j .text -j .rodata -j .data main.elf main.bin
	objcopy -O binary -j .text -j .rodata -j .data hallo.elf hallo.bin
	objcopy -O binary -j .text -j .rodata -j .data arith.elf arith.bin
	objcopy -O binary -j .text -j .rodata -j .data gp.elf gp.bin
	echo '0' > null.bin
	../lib_vm/combine_rom -o $@ \
		0x0000 main.bin \
		0x2000 hallo.bin \
		0x3000 arith.bin \
		0x4000 gp.bin \
		0x4ffffe null.bin
	$(RM) null.bin

../lib_vm/combine_rom: ../lib_vm/combine_rom.c
	$(MAKE) -C $(dir $@) $@

distclean::
	rm -f cdrom.iso
