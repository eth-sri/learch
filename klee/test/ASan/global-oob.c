// RUN: clang global-oob.c -fsanitize=address -emit-llvm -g -c -o global-oob-c.bc
// RUN: rm -rf global-oob-c.klee-out
// RUN: klee --output-dir=global-oob-c.klee-out global-oob-c.bc

int global_array[100] = {-1};
int main(int argc, char **argv) {
  return global_array[argc + 100];  // BOOM
}