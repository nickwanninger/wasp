%macro trial 1
	mov edi, (0x0000 + (%1 * 8))
	rdtsc
	mov [edi], eax
	mov [edi+4], edx
%endmacro


%macro hcall 1
	mov eax, %1
	out 0xFF, eax
%endmacro

HCALL_TEST equ 0
HCALL_EXIT equ 1

[bits 16]
[section .text]

extern stack_start

global _start
_start:
	; baseline measurement
	trial 0
	trial 1


	cli
	trial 2

	mov eax, gdtr32
	lgdt [eax]

	trial 3

	mov eax, cr0
	or al, 1
	mov cr0, eax

	trial 4

	jmp 08h:main


[section .data]
gdt32:
  	dq 0x0000000000000000
  	dq 0x00cf9a000000ffff
  	dq 0x00cf92000000ffff
gdtr32:
	dw 23
	dd gdt32



[bits 32]
[section .text]
main:

	trial 5
	mov esp, stack_start

	push ebp
  mov ebp, esp




	hcall HCALL_EXIT
