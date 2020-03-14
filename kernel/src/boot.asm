extern kmain ;; c entry point
extern record_timestamp


extern boot_stack_end


; MSR numbers
MSR_EFER equ 0xC0000080

; EFER bitmasks
EFER_LM equ 0x100
EFER_NX equ 0x800

; CR4 bitmasks
CR4_PAE equ 0x20
CR4_PSE equ 0x10


; CR0 bitmasks
CR0_PAGING equ 0x80000000

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



;; statically allocate page mappings
align PAGE_SIZE
[global page_directory]
page_directory:
	dq (boot_p3 + PG_PRESENT + PG_WRITABLE)
	times 511 dq 0

boot_p3:
	dq (boot_p2 + PG_PRESENT + PG_WRITABLE)
	times 511 dq 0

;; id map the first 512 2mb regions
boot_p2:
	;; pg starts at zero
	%assign pg 0
	;; repeat 512 times
  %rep 512
		;; store the mapping to the page
    dq (pg | PG_BIG | PG_PRESENT | PG_WRITABLE)
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

section .boot

;; entrypoint
[bits 64]
global _start
_start:
jmp start64

;;[bits 32]
;;
;;start32:
;;	;; load up the boot stack
;;	mov esp, boot_stack_end
;;
;;	;; move the info that grub passes into the kenrel into
;;	;; arguments that we will use when calling kmain later
;;	mov edi, ebx
;;	mov esi, eax
;;
;;	;; BASELINE
;;	call record_timestamp
;;
;;  ; enable PAE and PSE
;;  mov eax, cr4
;;  or eax, (CR4_PAE + CR4_PSE)
;;  mov cr4, eax
;;
;;	;; PAE + PSE
;;	call record_timestamp
;;
;;	call kmain
;;
;;	; enable long mode and the NX bit
;;  mov ecx, MSR_EFER
;;  rdmsr
;;  or eax, (EFER_LM + EFER_NX)
;;  wrmsr
;;
;;	;; load the page directory into cr3
;;  mov eax, page_directory
;;  mov cr3, eax
;;
;;  ; enable paging
;;  mov eax, cr0
;;  or eax, CR0_PAGING
;;  mov cr0, eax
;;
;;
;;  ; leave compatibility mode and enter long mode
;;  lgdt [gdtr]
;;
;;  mov ax, 0x10
;;  mov ss, ax
;;  mov ax, 0x0
;;  mov ds, ax
;;  mov es, ax
;;  mov fs, ax
;;  mov gs, ax
;;
;;.spn:
;;	jmp .spn


[bits 64]
start64:
	;; load up the boot stack
	mov esp, boot_stack_end

	;; BASELINE
	call record_timestamp

    ;; jump into kernel
    call kmain
    hlt
