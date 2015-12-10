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

SSRC := ../../lib_real/boot-cdrom.S \
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

boot-cdrom: boot-cdrom.elf
	objcopy -O binary -j .text -j .rodata -j .data $< $@

cdrom.iso: boot-cdrom
	mkdir cdrom.dir
	mkdir cdrom.dir/isolinux
	cp boot-cdrom cdrom.dir/isolinux/isolinux.bin
	genisoimage -r -T -b isolinux/isolinux.bin -c isolinux/boot.cat \
		-no-emul-boot -boot-info-table -o cdrom.iso cdrom.dir
	rm -rf cdrom.dir

distclean::
	rm -f cdrom.iso
