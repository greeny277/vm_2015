SECTIONS
{
        . = 0xe000;
        .rom : AT (0) {
                *(.text)
        }
}
