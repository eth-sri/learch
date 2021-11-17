// RUN: clang stack-oob.c -fsanitize=address -emit-llvm -g -c -o stack-oob-c.bc
// RUN: rm -rf stack-oob-c.klee-out
// RUN: klee --output-dir=stack-oob-c.klee-out stack-oob-c.bc

int main(int argc, char **argv) {
  int stack_array[100];
  stack_array[1] = 0;
  return stack_array[argc + 100];  // BOOM
}