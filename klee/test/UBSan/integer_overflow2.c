// RUN: clang integer_overflow2.c -fsanitize=signed-integer-overflow -emit-llvm -g -c -o integer_overflow2.bc
// RUN: rm -rf integer_overflow2.klee-out
// RUN: klee --output-dir=integer_overflow2.klee-out integer_overflow2.bc

#include "klee/klee.h"

int main() {
  unsigned int x;
  unsigned int y;
  volatile unsigned int result;

  klee_make_symbolic(&x, sizeof(x), "x");
  klee_make_symbolic(&y, sizeof(y), "y");

  result = x + y;
  result = x - y;
  result = x * y;
  result = x / y;
  result = x % y;
  result = -x;

  return 0;
}