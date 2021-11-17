// RUN: clang memory-leak.c -fsanitize=address -emit-llvm -g -c -o memory-leak-c.bc
// RUN: rm -rf memory-leak-c.klee-out
// RUN: klee --output-dir=memory-leak-c.klee-out memory-leak-c.bc

#include <stdlib.h>

void *p;

int main() {
  p = malloc(7);
  p = 0; // The memory is leaked here.
  return 0;
}