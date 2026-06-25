default rel
%include "/home/gardina_elizaveta/projects/1sem/language/source/backend_x64/mystdlib.asm"
section .text


square:
    push rbp
    mov rbp, rsp
    sub rsp, 48
    movsd [rbp-8], xmm0
    movsd xmm0, [rbp-8]
    sub rsp, 8
    movsd [rsp], xmm0

    movsd xmm0, [rbp-8]
    movsd xmm1, [rsp]
    add rsp, 8

    mulsd xmm1, xmm0
    movapd xmm0, xmm1
    movsd [rbp-16], xmm0
    movsd xmm0, [rbp-16]

    mov rsp, rbp
    pop rbp
    ret
    mov rsp, rbp
    pop rbp
    ret
global main

main:
    push rbp
    mov rbp, rsp
    sub rsp, 48

    movsd xmm0, [rel const_0]
    movsd [rbp-8], xmm0
    call square
    movsd [rbp-16], xmm0
    movsd xmm0, [rbp-16]

    mov rdi, fmt_print
    mov eax, 1
    call printf

    xor eax, eax
    mov rsp, rbp
    pop rbp
    ret

section .data
fmt_print: db "%.3g", 10, 0
fmt_scan: db "%lf", 0

const_0 dq 5.000000

