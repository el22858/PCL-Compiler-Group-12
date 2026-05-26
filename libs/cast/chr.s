    global _chr

    section .text
_chr:
    push    rbp
    mov     rbp, rsp
    mov     ax, di
    and     rax, 0xff
    pop     rbp
    ret