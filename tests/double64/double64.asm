[bits 64]
[section .text]

extern stack_start

global _start
_start:
	;; double the number
	shl rax, 1
	hlt
