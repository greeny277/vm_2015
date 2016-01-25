OUTPUT_FORMAT("elf32-i386")

SECTIONS
{
	. = 0xe000;

	text = .;

	.text : {
		*(.text)
	}

	etext = .;

	.rodata : {
		*(.rodata) *(.rodata.*)
	}

	end = .;
}
