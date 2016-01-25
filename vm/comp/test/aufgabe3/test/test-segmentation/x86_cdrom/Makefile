#
# $Id: Makefile 10011 2014-10-15 16:08:01Z raimue $
#

all: cdrom.iso

SIMULATION_PC := pc-1.vhdl
MEDIA_IMAGE := cdrom.iso
CHANGE_DISK_SIZE := 32
CHANGE_MEM_SIZE := 64

include $(dir $(shell which faum))/../share/faumachine/vhdl/experiment.mk

AS=as -32
LD=ld -m elf_i386

SSRC := boot-cdrom.S \
	../../lib_real/gfx_libs.S \
	../main.S \
	../../lib_real/kbd_ctrl.S

OBJS := $(SSRC:.S=.o)
SSRC := $(addprefix $(EXP_DIR),$(SSRC))

%.o: $(EXP_DIR)/%.S
	$(AS) -o $*.o $(EXP_DIR)/$*.S

boot-cdrom.elf: $(EXP_DIR)/boot.lds $(OBJS)
	$(LD) -T $(EXP_DIR)/boot.lds -o $@ $(OBJS)

clean distclean::
	$(RM) *.o
	$(RM) -f boot-cdrom.elf
	$(RM) boot-cdrom
	$(RM) ../../lib_real/*.o
	$(RM) *.bin
	$(RM) *.elf

main.elf: ../main.o ../../lib_real/gfx_libs.o ../../lib_real/kbd_ctrl.o
	$(LD) -T $(EXP_DIR)/process.lds -o $@ $^

hallo.elf: ../hallo.o ../../lib_real/linux_libs.o
	$(LD) -T $(EXP_DIR)/process.lds -o $@ $^

arith.elf: ../arith.o ../../lib_real/linux_libs.o
	$(LD) -T $(EXP_DIR)/process.lds -o $@ $^

gp.elf: ../gp.o
	$(LD) -T $(EXP_DIR)/process.lds -o $@ $^

boot-cdrom: boot-cdrom.elf \
		main.elf \
		hallo.elf \
		arith.elf \
		gp.elf \
		../../lib_vm/combine_rom
	objcopy -O binary -j .text -j .rodata -j .data boot-cdrom.elf bc.bin
	objcopy -O binary -j .text -j .rodata -j .data main.elf main.bin
	objcopy -O binary -j .text -j .rodata -j .data hallo.elf hallo.bin
	objcopy -O binary -j .text -j .rodata -j .data arith.elf arith.bin
	objcopy -O binary -j .text -j .rodata -j .data gp.elf gp.bin
	../../lib_vm/combine_rom -o $@ \
		0x0000 bc.bin \
		0x1000 main.bin \
		0x3000 hallo.bin \
		0x4000 arith.bin \
		0x5000 gp.bin

../../lib_vm/combine_rom: ../../lib_vm/combine_rom.c
	$(MAKE) -C ../../lib_vm combine_rom

cdrom.iso: boot-cdrom
	mkdir cdrom.dir
	mkdir cdrom.dir/isolinux
	cp boot-cdrom cdrom.dir/isolinux/isolinux.bin
	genisoimage -r -T -b isolinux/isolinux.bin -c isolinux/boot.cat \
		-no-emul-boot -boot-info-table -o cdrom.iso cdrom.dir
	rm -rf cdrom.dir

distclean::
	rm -f cdrom.iso
