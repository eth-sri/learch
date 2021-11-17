// RUN: clang use-after-free.c -fsanitize=address -emit-llvm -g -c -o use-after-free-c.bc
// RUN: rm -rf use-after-free-c.klee-out
// RUN: klee --output-dir=use-after-free-c.klee-out use-after-free-c.bc

#include <stdlib.h>

int main(int argc, char **argv) {
  int *array = malloc(100 * sizeof(int));
  free(array);
  return array[argc];  // BOOM
}