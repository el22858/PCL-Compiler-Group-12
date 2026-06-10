
	global _writeBoolean
	extern printf

	section .text
_writeBoolean:
		push	rdi
		push	rcx
		mov	r8b, dil
		or	r8b, r8b
		jnz	isTrue
		mov	rsi, strFalse
		jmp	timeToPrint
isTrue:
		mov	rsi, strTrue
timeToPrint:
		mov	rdi, format
		call	printf WRT ..plt
		pop	rcx
		pop	rdi
		ret

strTrue:	db	"true", 0
strFalse:	db	"false", 0
format:		db	"%s", 0
