
%macro hypercall 2
out 0xFF, 0
%endmacro



global print
print:

  push ebp
  mov ebp, esp

	mov edi, DWORD [ebp+8]
	out 0xFF, eax

	add eax, edx
  pop ebp
	ret


global exit
exit:
	out 0xFA, eax ;; exit
	ret

global write
write:
  push ebp
  mov ebp, esp

	;; store callee-saved
	push esi
	push edi
	push edx

	;; all the arguments are on the stack (-m32)
	;; fd
	mov edi, DWORD [ebp + 8]
	;; buf
	mov esi, DWORD [ebp + 12]
	;; len
	mov edx, DWORD [ebp + 16]

	;; `hypercall' for write()
	mov eax, 1
	out 0xFF, eax


	;; load callee-saved
	pop edx
	pop edi
	pop esi

  pop ebp
	ret


global send
send:
  push ebp
  mov ebp, esp

	;; store callee-saved
	push esi
	push edi

	mov edi, DWORD [ebp + 8]
	mov esi, DWORD [ebp + 12]

	mov eax, 2
	out 0xFF, eax

	;; load callee-saved
	pop edi
	pop esi

  pop ebp
	ret


global recv
recv:
  push ebp
  mov ebp, esp

	;; store callee-saved
	push esi
	push edi

	mov edi, DWORD [ebp + 8]
	mov esi, DWORD [ebp + 12]

	mov eax, 3
	out 0xFF, eax

	;; load callee-saved
	pop edi
	pop esi

  pop ebp
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
