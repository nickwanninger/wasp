
%macro hypercall 2
out 0xFF, 0
%endmacro



global print
print:

  push rbp
  mov rbp, rsp

	mov edi, DWORD [ebp+8]
	out 0xFF, eax

	add eax, edx
  pop rbp
	ret


global exit
exit:
	out 0xFA, eax ;; exit
	ret

global write
write:
  push rbp
  mov rbp, rsp

	;; store callee-saved
	push rsi
	push rdi
	push rdx

	;; all the arguments are on the stack (-m32)
	;; fd
	;mov edi, DWORD [ebp + 8]
	;; buf
	;mov esi, DWORD [ebp + 12]
	;; len
	;mov edx, DWORD [ebp + 16]

    ;; in x64, all arguments are already in appropriate registers

	;; `hypercall' for write()
	mov eax, 1
	out 0xFF, eax


	;; load callee-saved
	pop rdx
	pop rdi
	pop rsi

  pop rbp
	ret


global send
send:
  push rbp
  mov rbp, rsp

	;; store callee-saved
	push rsi
	push rdi

    ; variables will already be in correct spot in x64
    ; rdi -> void *data (u64)
    ; esi -> int len (i32)
	;mov edi, DWORD [ebp + 8]
	;mov esi, DWORD [ebp + 12]

	mov eax, 2
	out 0xFF, eax

	;; load callee-saved
	pop rdi
	pop rsi

  pop rbp
	ret


global recv
recv:
  push rbp
  mov rbp, rsp

	;; store callee-saved
	push rsi
	push rdi

    ; variables will already be in correct spot in x64
    ; rdi -> void *data (u64)
    ; esi -> int len (i32)
	;mov edi, DWORD [ebp + 8]
	;mov esi, DWORD [ebp + 12]

	mov eax, 3
	out 0xFF, eax

	;; load callee-saved
	pop rdi
	pop rsi

  pop rbp
	ret




;; store timestamps in the first page
timestamp_address dq 0x1000


;; the record_timestamp function just records the current value from
;; `rdtsc` into the next 64bit location in the timestamp page

global record_timestamp
record_timestamp:
	;; read the time stamp counter ASAP
	rdtsc

	mov ecx, [timestamp_address]

	mov [ecx], eax
	mov [ecx + 4], edx

	add ecx, 8

	mov [timestamp_address], ecx
	;; zero out the next 64 bits
	mov dword [ecx], 0
	mov dword [ecx + 4], 0
	ret
