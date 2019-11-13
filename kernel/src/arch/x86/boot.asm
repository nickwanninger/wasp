

extern kmain ;; c entry point
[extern high_kern_end]
[extern _virt_start]
[extern _virt_end]


; Declare constants for the multiboot header.
MBALIGN  equ  1 << 0            ; align loaded modules on page boundaries
MEMINFO  equ  1 << 1            ; provide memory map
FLAGS    equ  MBALIGN | MEMINFO ; this is the Multiboot 'flag' field
MAGIC    equ  0x1BADB002        ; 'magic number' lets bootloader find the header
CHECKSUM equ -(MAGIC + FLAGS)   ; checksum of above, to prove we are multiboot


section .multiboot
align 8
	dd MAGIC
	dd FLAGS
	dd CHECKSUM






; MSR numbers
MSR_EFER equ 0xC0000080

; EFER bitmasks
EFER_LM equ 0x100
EFER_NX equ 0x800

; CR4 bitmasks
CR4_PAE equ 0x20
CR4_PSE equ 0x10

; page flag bitmasks
PG_PRESENT  equ 0x1
PG_WRITABLE equ 0x2
PG_USER     equ 0x4
PG_BIG      equ 0x80
PG_NO_EXEC  equ 0x8000000000000000

; page and table size constants
LOG_TABLE_SIZE equ 9
LOG_PAGE_SIZE  equ 12
PAGE_SIZE  equ (1 << LOG_PAGE_SIZE)
TABLE_SIZE equ (1 << LOG_TABLE_SIZE)

; bootstrap stack size and alignment
STACK_SIZE  equ 0x1000
STACK_ALIGN equ 16


; paging structures
align PAGE_SIZE
[global boot_p4]
boot_p4:
	dq (boot_p3 + PG_PRESENT + PG_WRITABLE)
	times 511 dq 0

boot_p3:
	dq (boot_p2 + PG_PRESENT + PG_WRITABLE)
	times 511 dq 0


boot_p2:
	dq (boot_p1_1 + PG_PRESENT + PG_WRITABLE)
	dq (boot_p1_2 + PG_PRESENT + PG_WRITABLE)
	times 510 dq 0


;; ID map the first bit 512 pages of memory
boot_p1_1:
	;; pg starts at zero
	%assign pg 0
	;; repeat 512 times
  %rep 512
		;; store the mapping to the page
    dq (pg + PG_PRESENT + PG_WRITABLE)
		;; pg += 4096 (small page size)
    %assign pg pg+PAGE_SIZE
  %endrep

boot_p1_2:
	%assign pg (512 * PAGE_SIZE)
	;; repeat 512 times
  %rep 512
		;; store the mapping to the page
    dq (pg + PG_PRESENT + PG_WRITABLE)
		;; pg += 4096 (small page size)
    %assign pg pg+PAGE_SIZE
  %endrep



; the global descriptor table
gdt:
  ; null selector
    dq 0
  ; cs selector
    dq 0x00AF98000000FFFF
  ; ds selector
    dq 0x00CF92000000FFFF
gdt_end:
  dq 0 ; some extra padding so the gdtr is 16-byte aligned


gdtr:
  dw gdt_end - gdt - 1
  dq gdt


bits 32
align 8


section .boot

;; Kernel entrypoint, 32bit code
global _start
_start:

	;; move the info that grub passes into the kenrel into
	;; arguments that we will use when calling kmain later
	mov edi, ebx
	mov esi, eax

  ; enable PAE and PSE
  mov eax, cr4
  or eax, (CR4_PAE + CR4_PSE)
  mov cr4, eax


	; enable long mode and the NX bit
  mov ecx, MSR_EFER
  rdmsr
  or eax, (EFER_LM + EFER_NX)
  wrmsr


  ; set cr3 to a pointer to pml4
  mov eax, boot_p4
  mov cr3, eax


  ; enable paging
  mov eax, cr0
  or eax, 0x80000000
  mov cr0, eax

  ; leave compatibility mode and enter long mode
  lgdt [gdtr]


  mov ax, 0x10
  mov ss, ax
  mov ax, 0x0
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax






.spn:
	jmp .spn
