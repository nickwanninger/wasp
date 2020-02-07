
;; Just calls the workload 100 times.
[bits 16]
[section .text]

extern stack_start

global _start
_start:
	mov esp, stack_start
	mov ebp, esp
	hlt
