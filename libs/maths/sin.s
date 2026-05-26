    section .code
    global  _sin

_sin   push rbp
        mov rbp, rsp
        fld tword [rbp+16]
        fsin
        pop rbp
        ret