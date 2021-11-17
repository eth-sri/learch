clang double-free.c -fsanitize=address -emit-llvm -g -c -o double-free-c.bc
rm -rf double-free-c.klee-out
klee --output-dir=double-free-c.klee-out double-free-c.bc

clang global-oob.c -fsanitize=address -emit-llvm -g -c -o global-oob-c.bc
# rm -rf global-oob-c.klee-out
# klee --output-dir=global-oob-c.klee-out global-oob-c.bc

clang heap-oob.c -fsanitize=address -emit-llvm -g -c -o heap-oob-c.bc
# rm -rf heap-oob-c.klee-out
# klee --output-dir=heap-oob-c.klee-out heap-oob-c.bc

clang heap-oob.cpp -fsanitize=address -emit-llvm -g -c -o heap-oob-cpp.bc
# rm -rf heap-oob-cpp.klee-out
# klee --output-dir=heap-oob-cpp.klee-out heap-oob-cpp.bc

clang memory-leak.c -fsanitize=address -emit-llvm -g -c -o memory-leak-c.bc
# rm -rf memory-leak-c.klee-out
# klee --output-dir=memory-leak-c.klee-out memory-leak-c.bc

clang stack-oob.c -fsanitize=address -emit-llvm -g -c -o stack-oob-c.bc
# rm -rf stack-oob-c.klee-out
# klee --output-dir=stack-oob-c.klee-out stack-oob-c.bc

clang use-after-free.c -fsanitize=address -emit-llvm -g -c -o use-after-free-c.bc
# rm -rf use-after-free-c.klee-out
# klee --output-dir=use-after-free-c.klee-out use-after-free-c.bc

clang use-after-free.c -fsanitize=address -emit-llvm -g -c -o use-after-free-cpp.bc
# rm -rf use-after-free-cpp.klee-out
# klee --output-dir=use-after-free-cpp.klee-out use-after-free-cpp.bc

rm -rf *.bc
rm -rf *.klee-out