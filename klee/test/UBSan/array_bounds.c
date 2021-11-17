// RUN: clang array_bounds.c -fsanitize=bounds -emit-llvm -g -c -o array_bounds.bc
// RUN: rm -rf array_bounds.klee-out
// RUN: klee --output-dir=array_bounds.klee-out array_bounds.bc

#include "klee/klee.h"

unsigned int array_index(unsigned int n) {
  unsigned int a[4] = {0};

  return a[n];
}

int main() {
  unsigned int x;
  volatile unsigned int result;

  klee_make_symbolic(&x, sizeof(x), "x");

  result = array_index(x);

  return 0;
}