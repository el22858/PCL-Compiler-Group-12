    section .code
    global  _tan

_tan   push rbp
        mov rbp, rsp
        fld tword [rbp+16]
        fptan
        ffree
        fincstp
        pop rbp
        ret