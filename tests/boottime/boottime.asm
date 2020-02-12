%define TRIAL(n) (0x0000 + (n * 8))

%macro trial 1
	mov edi, (0x0000 + (%1 * 8))
	rdtsc
	mov [edi], eax
	mov [edi+4], edx
%endmacro


[bits 16]
[section .text]

extern stack_start

global _start
_start:

	; baseline
	trial 0
	; cli

	trial 1

	mov eax, 0
	out 0xFF, eax


	trial 2



	mov eax, 1
	out 0xFF, eax

	hlt

