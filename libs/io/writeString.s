	global writeString
	extern printf

	section .text
writeString:
	push	rax
	push	rcx
	push	rdi
	xor		rax, rax
	call	printf WRT ..plt
	pop		rdi
	pop		rcx
	pop		rax
	ret
format:	db "%s", 0