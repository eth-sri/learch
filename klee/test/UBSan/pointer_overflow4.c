// REQUIRES: geq-llvm-10.0

// RUN: clang pointer_overflow4.c -fsanitize=pointer-overflow -emit-llvm -g -c -o pointer_overflow4.bc
// RUN: rm -rf pointer_overflow4.klee-out
// RUN: klee --output-dir=pointer_overflow4.klee-out pointer_overflow4.bc

#include "klee/klee.h"
#include <stdio.h>

int main() {
  char c;
  char* ptr = &c;

  size_t offset;
  volatile char* result;

  klee_make_symbolic(&offset, sizeof(offset), "offset");
  klee_assume((size_t)(ptr) + offset != 0);

  result = ptr + offset;

  return 0;
}