// REQUIRES: geq-llvm-12.0

// RUN: clang shift3.c -fsanitize=shift -emit-llvm -g -c -o shift3.bc
// RUN: rm -rf shift3.klee-out
// RUN: klee --output-dir=shift3.klee-out shift3.bc

#include "klee/klee.h"

int lsh_overflow(unsigned int a, unsigned int b) {
  return a << b;
}

int main() {
  unsigned int a;
  unsigned int b;
  volatile unsigned int result;

  klee_make_symbolic(&a, sizeof(a), "a");
  klee_make_symbolic(&b, sizeof(b), "b");

  result = lsh_overflow(a, b);

  return 0;
}