OUTPUT_FORMAT("elf32-i386")

SECTIONS
{
	. = 0x7c00;

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
