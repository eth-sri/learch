
// RUN: clang integer_overflow1.c -fsanitize=signed-integer-overflow -emit-llvm -g -c -o integer_overflow1.bc
// RUN: rm -rf integer_overflow1.klee-out
// RUN: klee --output-dir=integer_overflow1.klee-out integer_overflow1.bc

#include "klee/klee.h"

int main() {
  signed int x;
  signed int y;
  volatile signed int result;

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