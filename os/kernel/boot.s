; Multiboot2 header and 64-bit setup
[bits 32]
%define MB2_MAGIC 0xE85250D6
%define MB2_ARCH 0
%define MB2_LENGTH header_end - header_start
%define MB2_CHECKSUM -(MB2_MAGIC + MB2_ARCH + MB2_LENGTH)

section .multiboot
header_start:
    dd MB2_MAGIC
    dd MB2_ARCH
    dd MB2_LENGTH
    dd MB2_CHECKSUM
    align 8
    dw 0 ; end tag type
    dw 0
    dd 8
header_end:

section .bss
    align 16
stack_bottom:
    resb 16384
stack_top:

section .data
align 4096
global pml4
pml4:
    dq pdpt + 0x3
align 4096
pdpt:
    dq pd + 0x3
align 4096
pd:
    dq pt + 0x3
    times 511 dq 0
align 4096
pt:
    %assign i 0
    %rep 512
        dq i*0x1000 | 0x3
        %assign i i+1
    %endrep

section .data
    gdt:
        dq 0
        dq 0x00af9a000000ffff
        dq 0x00cf92000000ffff
    gdt_ptr:
        dw gdt_end - gdt -1
        dq gdt
    gdt_end:

section .text
global start
start:
    mov esp, stack_top
    lgdt [gdt_ptr]

    mov eax, pml4
    mov cr3, eax
    mov eax, cr4
    or eax, 0x20
    mov cr4, eax
    mov ecx, 0xC0000080
    rdmsr
    or eax, 0x100
    wrmsr
    mov eax, cr0
    or eax, 0x80000001
    mov cr0, eax
    jmp 0x08:long_mode

align 16
long_mode:
[bits 64]
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov rsp, stack_top
    xor rbp, rbp
    extern kernel_main
    call kernel_main

.hang:
    hlt
    jmp .hang
