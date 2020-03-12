[bits 64]
[section .text]

extern stack_start

global _start
_start:
    mov eax, 0
	out 0xFF, eax

	;; double the number
	shl rax, 1
	hlt
