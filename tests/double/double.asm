[bits 16]
[section .text]

extern stack_start

global _start
_start:

	mov ax, 0
	mov sp, ax
	mov ds, ax

	;; get number
	mov eax, 0
	out 0xFF, eax

	;; double the number
	add ebx, ebx

	;; verify
	mov eax, 1
	out 0xFF, eax

	;; exit
	mov eax, 2
	out 0xFF, eax

	hlt
