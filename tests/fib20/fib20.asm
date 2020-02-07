[bits 16]
[section .text]

extern stack_start

global _start
_start:
	cli
	mov eax, gdtr32
	lgdt [eax]
	mov eax, cr0
	or al, 1
	mov cr0, eax

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

	mov eax, 0
	mov ds, ax

	mov esp, stack_start

	push ebp
  mov ebp, esp

	push 20
	call fib
	add esp, 4
	out 0xFF, eax

  mov eax, 0
  leave
  ret

fib:
  push esi
  push ebx
  sub esp, 4
  mov eax, DWORD [esp+16]
  cmp eax, 1
  jle .L1
  lea ebx, [eax-2]
  xor esi, esi
.L3:
  sub esp, 12
  push ebx
  sub ebx, 1
  call fib
  add esp, 16
  add esi, eax
  cmp ebx, -1
  jne .L3
  lea eax, [esi+1]
.L1:
  add esp, 4
  pop ebx
  pop esi
  ret
