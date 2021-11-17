// RUN: clang shift2.c -fsanitize=shift -emit-llvm -g -c -o shift2.bc
// RUN: rm -rf shift2.klee-out
// RUN: klee --output-dir=shift2.klee-out shift2.bc

#include "klee/klee.h"

int rsh_inbounds(signed int a, signed int b) {
  return a >> b;
}

int main() {
  signed int a;
  signed int b;
  volatile signed int result;

  klee_make_symbolic(&a, sizeof(a), "a");
  klee_make_symbolic(&b, sizeof(b), "b");

  result = rsh_inbounds(a, b);

  return 0;
}