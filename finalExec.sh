#!/usr/bin/env bash

IN="$1"
temp="${IN##*/}"
final="${temp%.*}"
./pcl "$@"
nasm -f elf64 "./${final}.asm"
gcc "${final}.o" "lib.a"
