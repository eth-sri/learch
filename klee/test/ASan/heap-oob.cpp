// RUN: clang heap-oob.cpp -fsanitize=address -emit-llvm -g -c -o heap-oob-cpp.bc
// RUN: rm -rf heap-oob-cpp.klee-out
// RUN: klee --output-dir=heap-oob-cpp.klee-out heap-oob-cpp.bc

int main(int argc, char **argv) {
  int *array = new int[100];
  array[0] = 0;
  int res = array[argc + 100];  // BOOM
  delete [] array;
  return res;
}