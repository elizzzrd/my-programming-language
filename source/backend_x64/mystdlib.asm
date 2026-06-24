extern printf
extern scanf
extern exit

extern pow
extern sin 
extern cos
extern tan
extern log
extern sqrt
extern exp

section .data
fmt_double      db "%g", 10, 0
fmt_double_in   db "%lg", 0

section .text

;--------------------------------------------------------------
;   _my_exit
;--------------------------------------------------------------
global _my_exit
_my_exit:
    mov rdi, 0
    call exit


;--------------------------------------------------------------
;   _my_printf
;--------------------------------------------------------------
global _my_printf
_my_printf:
    push rbp
    mov rbp, rsp
    sub rsp, 32

    lea rdi, [fmt_double]
    mov rax, 1
    call printf

    mov rsp, rbp
    pop rbp
    ret


;--------------------------------------------------------------
;   _my_read
;--------------------------------------------------------------
global _my_read:
_my_read:
    push rbp
    mov rbp, rsp
    sub rsp, 40

    sub rsp, 8
    lea rsi, [rsp]
    lea rdi, [fmt_double_in]
    xor rax, rax
    call scanf

    movsd xmm0, [rsp]
    add rsp, 8

    mov rsp, rbp
    pop rbp
    ret    


