    global _pi

    section .text
_pi:
    push    rbp
    mov     rbp, rsp
    fldpi
    pop     rbp
    ret