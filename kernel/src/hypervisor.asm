
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
