    section .code
    global  _cos

_cos   push rbp
        mov rbp, rsp
        fld tword [rbp+16]
        fcos
        pop rbp
        ret