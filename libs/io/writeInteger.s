; Needs parameter to be moved to rax prior to the call

	global writeInteger
	extern printf

	section .text
writeInteger:
	push	rax
	push	rcx
	push	rdi
	mov	rdi, format
	mov	rsi, rax
	xor	rax, rax
	call	printf WRT ..plt
	pop	rdi
	pop	rcx
	pop	rax
	ret
format:
	db	"%ld", 0
