Obtaining the Benchmarks
=============================================================================================================
We compile each program to three versions: LLVM bitcode file, UBSan bitcode file, and gcov binary. For measuring coverage, we run Learch on the LLVM bitcode file to generate tests, replay the tests on the gcov binary, and measure coverage on the root build directory of the gcov binary. For detecting UBSan violations, we run Learch on the UBSan bitcode file.

## Coreutils
We used 103 coreutils program as benchmarks. They are listed in [`coreutils_all.txt`](coreutils_all.txt). The 51 in [`coreutils_train.txt`](coreutils_train.txt) are used for training and the 52 in [`coreutils_test.txt`](coreutils_all.txt) are used for testing. You can build the coreutils programs with the following command:
```
./prepare_coreutils.sh ${SOURCE_DIR}/benchmarks/coreutils_all.txt
```

## 10 Real-world Programs
We used 10 real-world programs for testing. They can be built with the following command:
```
./prepare_realworld.sh
```

[`realworld.txt`](realworld.txt) lists useful information about the real-world programs (each line lists the program name, path to LLVM bitcode file, path to UBSan bitcode file, path to gcov binary, gcov directory, and KLEE symbolic arguments, separated by `##`).