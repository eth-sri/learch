// RUN: clang shift1.c -fsanitize=shift -emit-llvm -g -c -o shift1.bc
// RUN: rm -rf shift1.klee-out
// RUN: klee --output-dir=shift1.klee-out shift1.bc

#include "klee/klee.h"

int lsh_overflow(signed int a, signed int b) {
  return a << b;
}

int main() {
  signed int a;
  signed int b;
  volatile signed int result;

  klee_make_symbolic(&a, sizeof(a), "a");
  klee_make_symbolic(&b, sizeof(b), "b");

  result = lsh_overflow(a, b);

  return 0;
}