    global _abs

    section .text
_abs:
    push    rbp
    mov     rbp, rsp
    mov     ax, di
    or      ax, ax
    jge     ok
    neg     ax
ok:
    and     rax, 0xffff
    pop     rbp
    ret
