global timer_isr_stub
global page_fault_isr_stub
extern timer_isr
extern page_fault_isr

section .text
bits 64
timer_isr_stub:
    push rbp
    mov rbp, rsp
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    call timer_isr
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    pop rbp
    iretq

page_fault_isr_stub:
    push rbp
    mov rbp, rsp
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    call page_fault_isr
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    add rsp, 8
    pop rbp
    iretq
