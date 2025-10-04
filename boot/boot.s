// Constants for multiboot header
.set ALIGN,    1<<0
.set MEMINFO,  1<<1
.set FLAGS,    ALIGN | MEMINFO
.set MAGIC,    0x1BADB002
.set CHECKSUM,  -(MAGIC + FLAGS)

// Multiboot header
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

//Creates stack
.section .bss
.align 16
stack_bottom:
.skip 16384
stack_top:

// Loads the kernel
.section .text
.global _start
.type _start, @function

_start:
    mov $stack_top, %esp

    mov %eax, %edi
    mov %ebx, %esi
    push %esi
    push %edi

    call kernel_main

    cli
1:  hlt
    jmp 1b

.size _start, . - _start
