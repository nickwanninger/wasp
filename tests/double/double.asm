[bits 16]
[section .text]


extern stack_start

global _start
_start:
	mov esp, stack_start
	mov ebp, esp

	imul eax, 2
	hlt
