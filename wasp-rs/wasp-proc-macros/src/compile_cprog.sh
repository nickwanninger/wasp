#!/bin/bash


# This file takes a nasm bootloader and c source code as two arguments and compiles them together
# to produce binary data as output.


boot=$1
shift

src=$1
shift

linker=$1
shift

cargs=$1
shift

tmp=$(mktemp -d)

# compile the c program
gcc -ffreestanding $cargs -fno-tree-vectorize -c $src -o $tmp/source.o

nasm -f elf32 $boot -o $tmp/boot.o

ld -T $linker -o $tmp/out.elf $tmp/boot.o $tmp/source.o
objcopy -O binary $tmp/out.elf $tmp/out.bin

cat $tmp/out.bin
