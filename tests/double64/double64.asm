[bits 64]
[section .text]

extern stack_start

global _start
_start:
	mov rax, 0
	mov rsp, rax

	;; get number
	mov eax, 0
	out 0xFF, eax

	;; double the number
	shl rbx, 1

	;; verify
	mov eax, 1
	out 0xFF, eax

	;; exit
	mov eax, 2
	out 0xFF, eax

	hlt
