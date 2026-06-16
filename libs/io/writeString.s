;     section .code
;     global _writeString
; 
; _writeString:   push rbp
;                 mov rbp, rsp
;                 push rdi
;                 push rsi
; 
; count:          cmp byte [rsi], 0x0
;                 jz print
;                 inc rax
;                 inc rsi
;                 jmp count
; 
; print:          mov rsi, rdi
;                 mov rdi, 1
;                 mov rdx, rax
;                 syscall
;                 
; done:
;                 pop rsi
;                 pop rdi
;                 pop rbp
;                 ret

        global _writeString
        extern puts
        extern printf

        section .text
_writeString:
;        call    puts WRT ..plt
;        ret
        ;pop	rax
		mov	rdi, format
		mov	rsi, rax
		xor	rax, rax
		call	printf WRT ..plt
		;pop		rax
		ret


format:         db "%s", 0 