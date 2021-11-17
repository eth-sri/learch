#!/bin/bash

clang array_bounds.c -I/home/hej/klee/ml-klee/include -fsanitize=bounds -emit-llvm -g -c -o array_bounds.bc
rm -rf array_bounds.klee-out
klee --output-dir=array_bounds.klee-out array_bounds.bc

clang null.c -I/home/hej/klee/ml-klee/include -fsanitize=null -emit-llvm -g -c -o null.bc
rm -rf null.klee-out
klee --output-dir=null.klee-out null.bc

clang pointer_overflow1.c -I/home/hej/klee/ml-klee/include -fsanitize=pointer-overflow -emit-llvm -g -c -o pointer_overflow1.bc
rm -rf pointer_overflow1.klee-out
klee --output-dir=pointer_overflow1.klee-out pointer_overflow1.bc

clang pointer_overflow2.c -I/home/hej/klee/ml-klee/include -fsanitize=pointer-overflow -emit-llvm -g -c -o pointer_overflow2.bc
rm -rf pointer_overflow2.klee-out
klee --output-dir=pointer_overflow2.klee-out pointer_overflow2.bc

clang pointer_overflow3.c -I/home/hej/klee/ml-klee/include -fsanitize=pointer-overflow -emit-llvm -g -c -o pointer_overflow3.bc
rm -rf pointer_overflow3.klee-out
klee --output-dir=pointer_overflow3.klee-out pointer_overflow3.bc

clang pointer_overflow4.c -I/home/hej/klee/ml-klee/include -fsanitize=pointer-overflow -emit-llvm -g -c -o pointer_overflow4.bc
rm -rf pointer_overflow4.klee-out
klee --output-dir=pointer_overflow4.klee-out pointer_overflow4.bc

clang shift1.c -I/home/hej/klee/ml-klee/include -fsanitize=shift -emit-llvm -g -c -o shift1.bc
rm -rf shift1.klee-out
klee --output-dir=shift1.klee-out shift1.bc

clang shift2.c -I/home/hej/klee/ml-klee/include -fsanitize=shift -emit-llvm -g -c -o shift2.bc
rm -rf shift2.klee-out
klee --output-dir=shift2.klee-out shift2.bc

clang shift3.c -I/home/hej/klee/ml-klee/include -fsanitize=shift -emit-llvm -g -c -o shift3.bc
rm -rf shift3.klee-out
klee --output-dir=shift3.klee-out shift3.bc

clang integer_overflow1.c -I/home/hej/klee/ml-klee/include -fsanitize=signed-integer-overflow -emit-llvm -g -c -o integer_overflow1.bc
rm -rf integer_overflow1.klee-out
klee --output-dir=integer_overflow1.klee-out integer_overflow1.bc

clang integer_overflow2.c -I/home/hej/klee/ml-klee/include -fsanitize=unsigned-integer-overflow -emit-llvm -g -c -o integer_overflow2.bc
rm -rf integer_overflow2.klee-out
klee --output-dir=integer_overflow2.klee-out integer_overflow2.bc