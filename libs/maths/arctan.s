    section .code
    global  _arctan

_arctan   push rbp
        mov rbp, rsp
        fld tword [rbp+16]
        fld1
        fpatan
        pop rbp
        ret