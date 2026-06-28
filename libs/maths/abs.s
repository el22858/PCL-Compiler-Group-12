    global _abs

    section .text
_abs:
    push    rbp
    mov     rbp, rsp
    mov     eax, [rbp + 8]
    ;mov     eax, di
    or      eax, eax
    jge     ok
    neg     eax
ok:
    and     eax, 0xffffffff
    mov     [rbp + 4], eax
    pop     rbp
    ret
