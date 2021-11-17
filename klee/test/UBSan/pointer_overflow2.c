// REQUIRES: geq-llvm-10.0

// RUN: clang pointer_overflow2.c -fsanitize=pointer-overflow -emit-llvm -g -c -o pointer_overflow2.bc
// RUN: rm -rf pointer_overflow2.klee-out
// RUN: klee --output-dir=pointer_overflow2.klee-out pointer_overflow2.bc

#include "klee/klee.h"
#include <stdio.h>

int main() {
  size_t address;
  volatile char *result;

  klee_make_symbolic(&address, sizeof(address), "address");

  char *ptr = (char *)address;

  result = ptr + 1;
  return 0;
}