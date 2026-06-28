; Needs parameter to be moved to rax prior to the call

	global _writeInteger
	extern printf

	section .text
_writeInteger:
	push	rax
	push	rcx
	push	rdi
	mov	rdi, format
	mov	rsi, rax
	;and rax, 1 << 31
	;jz timeToPrint
	;xor rsi, 0xffffffff
	;sub rsi, 0xfffffffd
timeToPrint:
	xor	rax, rax
	call	printf WRT ..plt
	pop	rdi
	pop	rcx
	pop	rax
	ret
format:
	db	"%d", 0
