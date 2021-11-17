// RUN: clang double-free.c -fsanitize=address -emit-llvm -g -c -o double-free-c.bc
// RUN: rm -rf double-free-c.klee-out
// RUN: klee --output-dir=double-free-c.klee-out double-free-c.bc

#include <stdlib.h>

int main(int argc, char **argv) {
  int *array = malloc(100 * sizeof(int));
  free(array);
  free(array);  // BOOM
  return 0;
}