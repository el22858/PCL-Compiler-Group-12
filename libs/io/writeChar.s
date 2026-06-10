        global _writeChar
        extern printf

        section .text
_writeChar:
        push    rax
        push    rcx
        push    rdi

        mov     rdi, format
        mov     rsi, rax
        xor     rax, rax
        call    printf WRT ..plt

        pop     rdi
        pop     rcx
        pop     rax
        ret
format: db      "%c", 0