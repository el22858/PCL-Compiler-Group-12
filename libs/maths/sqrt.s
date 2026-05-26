    section .code
    global  _sqrt

_sqrt   push rbp
        mov rbp, rsp
        fld tword [rbp+16]
        fsqrt
        pop rbp
        ret