%macro trial 1
	rdtsc
	mov edi, (0x0000 + (%1 * 8))
	mov [edi], eax
	mov [edi+4], edx
%endmacro


%macro hcall 1
	mov eax, %1
	out 0xFF, eax
%endmacro

HCALL_TEST equ 0
HCALL_EXIT equ 1


; CR0 bitmasks
CR0_PAGING equ 0x80000000

; CR4 bitmasks
CR4_PAE equ 0x20
CR4_PSE equ 0x10

; MSR numbers
MSR_EFER equ 0xC0000080

; EFER bitmasks
EFER_LM equ 0x100
EFER_NX equ 0x800



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

	jmp 08h:main32

[bits 32]
[section .text]
main32:

	trial 5
	mov esp, stack_start

	push ebp
  mov ebp, esp

	; start setting up paging
	; PML4[0] -> PDPT
	mov eax, pdpt
	or  eax, 0x3      ; entry is present, rw
	mov ebx, pml4
	mov [ebx], eax

	; PDPT[0] -> PDT
	mov eax, pd
	or  eax, 0x3
	mov ebx, pdpt
	mov [ebx], eax

	; Identity map the first GB with 2MB pages
	mov ecx, 512
	mov edx, pd
	mov eax, 0x83 ; set PS bit also (PDE -> 2MB page)
.write_pde:
	mov [edx], eax
	add eax, 0x200000
	add edx, 0x8
	loop .write_pde

	; put pml4 address in cr3
	mov eax, pml4
	mov cr3, eax

  ; enable PAE and PSE
  mov eax, cr4
  or eax, (CR4_PAE + CR4_PSE)
  mov cr4, eax

	; enable long mode and the NX bit
  mov ecx, MSR_EFER
  rdmsr
  or eax, (EFER_LM)
  wrmsr

  ; enable paging
  mov eax, cr0
  or  eax, CR0_PAGING
  mov cr0, eax

	trial 6

	mov eax, gdtr64
	lgdt [eax]


	trial 7
	jmp 0x08:main64

[bits 64]
main64:

	mov rax, 0

	trial 8
	hcall HCALL_EXIT



[section .data]
align 8
gdt32:
  	dq 0x0000000000000000
  	dq 0x00cf9a000000ffff
  	dq 0x00cf92000000ffff
align 8
gdtr32:
	dw 23
	dd gdt32


align 8
gdt64:
    dq 0x0000000000000000 ; null
    dq 0x00af9a000000ffff ; code (note lme bit)
    dq 0x00af92000000ffff ; data (most entries don't matter)

align 8
gdtr64:
    dw 23
    dq gdt64



section .data
align 4096
pml4: times 4096 db 0
pdpt: times 4096 db 0
pd: times 4096 db 0



