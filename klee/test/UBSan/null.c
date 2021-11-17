// RUN: clang null.c -fsanitize=null -emit-llvm -g -c -o null.bc
// RUN: rm -rf null.klee-out
// RUN: klee --output-dir=null.klee-out null.bc

#include "klee/klee.h"

int main() {
  _Bool null;
  volatile int result;

  klee_make_symbolic(&null, sizeof(null), "null");

  int local = 0;
  int *arg = null ? 0x0 : &local;

  result = *arg;
  return 0;
}