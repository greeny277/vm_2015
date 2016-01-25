OUTPUT_FORMAT("elf32-i386")

SECTIONS
{
	. = 0x0000;

	text = .;

	.text : {
		*(.text)
	}

	etext = .;

	.rodata : {
		*(.rodata) *(.rodata.*)
	}
	.data : {
		*(.data)
	}

	edata = .;

	.bss : {
		*(.bss)
	}

	end = .;
}
