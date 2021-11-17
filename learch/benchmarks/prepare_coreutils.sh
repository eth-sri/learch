#!/bin/bash

PROG_LIST=${1} # need absolute path
NUM_MAKE_CORES=4

# download
wget https://ftp.gnu.org/gnu/coreutils/coreutils-8.31.tar.xz
tar -xf coreutils-8.31.tar.xz
cd coreutils-8.31/

# build LLVM bitcode files (without instrumentation, used for measuring coverage)
mkdir obj-llvm
cd obj-llvm
CC=wllvm FORCE_UNSAFE_CONFIGURE=1 CFLAGS="-g -O1 -Xclang -disable-llvm-passes -D__NO_STRING_INLINES -D_FORTIFY_SOURCE=0 -U__OPTIMIZE__" ../configure --disable-nls
make -j${NUM_MAKE_CORES}
cd src
find . -executable -type f | xargs -I '{}' extract-bc '{}'

# build LLVM bitcode files instrumented with UBSan
cd ../..
mkdir obj-ubsan
cd obj-ubsan
CC=wllvm FORCE_UNSAFE_CONFIGURE=1 CFLAGS="-g -O1 -Xclang -disable-llvm-passes -D__NO_STRING_INLINES -D_FORTIFY_SOURCE=0 -U__OPTIMIZE__ -fsanitize=signed-integer-overflow -fsanitize=unsigned-integer-overflow -fsanitize=shift -fsanitize=bounds -fsanitize=pointer-overflow -fsanitize=null" ../configure --disable-nls
make -j${NUM_MAKE_CORES}
cd src
find . -executable -type f | xargs -I '{}' extract-bc '{}'

# build binaries with gcov
# you can parallelize this part with standard parallelization tools like GNU parallel
cd ../..
cat ${PROG_LIST} | while read prog
do
    mkdir obj-gcov-${prog}
    cd obj-gcov-${prog}
    ../configure FORCE_UNSAFE_CONFIGURE=1 --disable-nls CFLAGS="-g -O0 -fprofile-arcs -ftest-coverage"
    make -j${NUM_MAKE_CORES}
done
