#!/usr/bin/env bash

IN="$1"
temp="${IN##*/}"
final="${temp%.*}"
./pcl "$IN" "$final"
nasm -f elf64 "./${final}.asm"
gcc "${final}.o" "lib.o"
