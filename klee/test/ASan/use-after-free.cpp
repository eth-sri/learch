// RUN: clang use-after-free.c -fsanitize=address -emit-llvm -g -c -o use-after-free-cpp.bc
// RUN: rm -rf use-after-free-cpp.klee-out
// RUN: klee --output-dir=use-after-free-cpp.klee-out use-after-free-cpp.bc

int main(int argc, char **argv) {
  int *array = new int[100];
  delete [] array;
  return array[argc];  // BOOM
}