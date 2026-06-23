default rel
global main

%include "/home/gardina_elizaveta/projects/1sem/language/source/backend_x64/mystdlib.asm"
section .text

main:
    push rbp
    mov rbp, rsp
    sub rsp, 64

    movsd xmm0, [rel const_0]


    movsd [rbp-8], xmm0

    movsd xmm0, [rel const_1]


    movsd [rbp-16], xmm0

    movsd xmm0, [rbp-8]

    sub rsp, 8
    movsd [rsp], xmm0

    movsd xmm0, [rbp-16]

    movsd xmm1, [rsp]
    add rsp, 8

    movapd xmm2, xmm0
    movapd xmm0, xmm1
    movapd xmm1, xmm2
    call pow
    movapd xmm1, xmm0
    movapd xmm0, xmm1

    movsd [rbp-24], xmm0

    movsd xmm0, [rbp-24]


    mov rdi, fmt_print
    mov al, 1
    call printf



    xor eax, eax
    mov rsp, rbp
    pop rbp
    ret



section .data
fmt_print: db "%.3g", 10, 0
fmt_scan: db "%lf", 0

const_0 dq 10.000000
const_1 dq 3.000000

