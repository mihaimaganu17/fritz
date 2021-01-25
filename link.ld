/* Linker for our asm code
 * Created at 25/01/2021
 * Written by m3m0ry
 */

SECTIONS
{
    /* Load address for aarch64 */
    . = 0x80000;
    /* KEEP keeps the symbols in the section, even if not referenced */
    .text : { KEEP(*(.text.boot)) *(.text .text.*) }
    .rodata : { *(.rodata .rodata.*)  }
    /* https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_chapter/ld_3.html#SEC13 */
    PROVIDE(_data = .);
    .data : { *(.data .data.* )}
    .bss (NOLOAD) : {
        . = ALIGN(16);
        __bss_start = .;
        *(.bss .bss.*)
        /* Where to place your uninitialized data */
        *(COMMON)
        __bss_end = .;
    }
    _end = .;
    /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}
__bss_size = (__bss_end - __bss_start)>>3;
