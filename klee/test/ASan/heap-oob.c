// RUN: clang heap-oob.c -fsanitize=address -emit-llvm -g -c -o heap-oob-c.bc
// RUN: rm -rf heap-oob-c.klee-out
// RUN: klee --output-dir=heap-oob-c.klee-out heap-oob-c.bc

#include <stdlib.h>

int main(int argc, char **argv) {
    int* array = (int*) malloc(100 * sizeof(int));
    array[0] = 0;
    int res = array[argc + 100];
    free(array);
    return res;
}