# "Bootloader" for our RaspPi3 Aarch64
# Created at 25/01/2020
# Written by m3m0ry

.section ".text.boot"

.global _start

_start:
    mrs     x1, mpidr_el1       // load cpuid, stop slave cores(as 64-bit)
    and     x1, x1, #3          // x1 = x1 % 4
    cbz     x1, 2f              // compare and branch on zero
1:  wfe
    b       1b
2:  // cpu id == 0

    // set stack before our code
    ldr     x1, =_start         // load from literal pool(replaced with _start address)
    mov     sp, x1

    // clear bss(for i=bss_size, i>bss_start, i--)
    ldr     x1, =__bss_start    // load bss start address
    ldr     w2, =__bss_size     // load bss size (w registers are 32-bit)
3:  cbz     w2, 4f              // if __bss_size == 0, branch to 4
    str     xzr, [x1], #8       // post-indexed -> store xzr in x1, then x1 = x1 + 8
    sub     w2, w2, #1          // w2 = w2 - 1
    cbnz    w2, 3b              // if w2 != 0, branch to 3

4:  bl      main                // branch with link to main, copies next instruction into LR
    b       1b                  // halt this core to

