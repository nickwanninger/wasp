#!/bin/bash


if [ -z "$1" ];
then
		echo "usage: tools/create_test.sh <name>"
		exit 1
fi


mkdir -p "tests/$1"
echo "srcs += $1.asm" >> tests/$1/Makefile
cat << EOF > tests/$1/$1.asm


[bits 16]
[section .text]

extern stack_start

global _start
_start:
	mov esp, stack_start
	mov ebp, esp


	hlt
EOF
