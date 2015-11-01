SECTIONS
{
        . = 0x1000;
        .disk : AT (0) {
                *(.text)
		*(.data)
		*(.rodata)
        }
}
