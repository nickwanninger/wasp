#!/usr/bin/env bash

mkdir -p build

for dir in $(ls -d tests/*); do
	echo "Building $dir"
	make --no-print-directory -f Makefile.tests build_test M=$dir -j
done
