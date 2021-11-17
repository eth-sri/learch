// REQUIRES: geq-llvm-10.0

// RUN: clang pointer_overflow1.c -fsanitize=pointer-overflow -emit-llvm -g -c -o pointer_overflow1.bc
// RUN: rm -rf pointer_overflow1.klee-out
// RUN: klee --output-dir=pointer_overflow1.klee-out pointer_overflow1.bc

#include "klee/klee.h"
#include <stdio.h>

int main() {
  size_t address;
  volatile char *result;

  klee_make_symbolic(&address, sizeof(address), "address");
  klee_assume(address != 0);

  char *ptr = (char *)address;

  result = ptr + 1;
  return 0;
}