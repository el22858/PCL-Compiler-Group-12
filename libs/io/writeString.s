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