NUM_MAKE_CORES=4

########################## diff ##########################
wget https://ftp.gnu.org/gnu/diffutils/diffutils-3.7.tar.xz
tar -xf diffutils-3.7.tar.xz
cd diffutils-3.7

mkdir obj-llvm
cd obj-llvm
CC=wllvm CFLAGS="-g -O1 -Xclang -disable-llvm-passes -D__NO_STRING_INLINES -D_FORTIFY_SOURCE=0 -U__OPTIMIZE__" ../configure --disable-nls --disable-largefile
make -j${NUM_MAKE_CORES}
cd src
find . -executable -type f | xargs -I '{}' extract-bc '{}'
cd ../..

mkdir obj-ubsan
cd obj-ubsan
CC=wllvm CFLAGS="-g -O1 -Xclang -disable-llvm-passes -D__NO_STRING_INLINES -D_FORTIFY_SOURCE=0 -U__OPTIMIZE__ -fsanitize=signed-integer-overflow -fsanitize=unsigned-integer-overflow -fsanitize=shift -fsanitize=bounds -fsanitize=pointer-overflow -fsanitize=null" ../configure --disable-nls --disable-largefile
make -j${NUM_MAKE_CORES}
cd src
find . -executable -type f | xargs -I '{}' extract-bc '{}'
cd ../..

mkdir obj-gcov
cd obj-gcov
CFLAGS="-g -fprofile-arcs -ftest-coverage -g -O0" ../configure --disable-nls --disable-largefile
make -j${NUM_MAKE_CORES}
cd ../..

########################## find ##########################
wget https://ftp.gnu.org/gnu/findutils/findutils-4.7.0.tar.xz
tar -xf findutils-4.7.0.tar.xz
cd findutils-4.7.0

mkdir obj-llvm
cd obj-llvm
CC=wllvm CFLAGS="-g -O1 -Xclang -disable-llvm-passes -D__NO_STRING_INLINES -D_FORTIFY_SOURCE=0 -U__OPTIMIZE__" ../configure --disable-nls --disable-largefile --disable-threads --without-selinux
make -j${NUM_MAKE_CORES}
cd find
find . -executable -type f | xargs -I '{}' extract-bc '{}'
cd ../..

mkdir obj-ubsan
cd obj-ubsan
CC=wllvm CFLAGS="-g -O1 -Xclang -disable-llvm-passes -D__NO_STRING_INLINES -D_FORTIFY_SOURCE=0 -U__OPTIMIZE__ -fsanitize=signed-integer-overflow -fsanitize=unsigned-integer-overflow -fsanitize=shift -fsanitize=bounds -fsanitize=pointer-overflow -fsanitize=null" ../configure --disable-nls --disable-largefile --disable-threads --without-selinux
make -j${NUM_MAKE_CORES}
cd find
find . -executable -type f | xargs -I '{}' extract-bc '{}'
cd ../..

mkdir obj-gcov
cd obj-gcov
CFLAGS="-g -fprofile-arcs -ftest-coverage -g -O0" ../configure --disable-nls --disable-largefile --disable-threads --without-selinux
make -j${NUM_MAKE_CORES}
cd ../..

########################## grep ##########################
wget https://ftp.gnu.org/gnu/grep/grep-3.6.tar.xz
tar -xf grep-3.6.tar.xz
cd grep-3.6

mkdir obj-llvm
cd obj-llvm
CC=wllvm CFLAGS="-g -O1 -Xclang -disable-llvm-passes -D__NO_STRING_INLINES -D_FORTIFY_SOURCE=0 -U__OPTIMIZE__" ../configure --disable-largefile --disable-threads --disable-nls --disable-perl-regexp --with-included-regex
make -j${NUM_MAKE_CORES}
cd src
find . -executable -type f | xargs -I '{}' extract-bc '{}'
cd ../..

mkdir obj-ubsan
cd obj-ubsan
CC=wllvm CFLAGS="-g -O1 -Xclang -disable-llvm-passes -D__NO_STRING_INLINES -D_FORTIFY_SOURCE=0 -U__OPTIMIZE__ -fsanitize=signed-integer-overflow -fsanitize=unsigned-integer-overflow -fsanitize=shift -fsanitize=bounds -fsanitize=pointer-overflow -fsanitize=null" ../configure --disable-largefile --disable-threads --disable-nls --disable-perl-regexp --with-included-regex
make -j${NUM_MAKE_CORES}
cd src
find . -executable -type f | xargs -I '{}' extract-bc '{}'
cd ../..

mkdir obj-gcov
cd obj-gcov
CFLAGS="-g -fprofile-arcs -ftest-coverage -O0" ../configure --disable-largefile --disable-threads --disable-nls --disable-perl-regexp --with-included-regex
make -j${NUM_MAKE_CORES}
cd ../..

########################## gawk ##########################
wget https://ftp.gnu.org/gnu/gawk/gawk-5.1.0.tar.xz
tar -xf gawk-5.1.0.tar.xz
cd gawk-5.1.0

mkdir obj-llvm
cd obj-llvm
CC=wllvm CFLAGS="-g -O1 -Xclang -disable-llvm-passes -D__NO_STRING_INLINES -D_FORTIFY_SOURCE=0 -U__OPTIMIZE__" ../configure --disable-mpfr --disable-largefile --disable-nls
make -j${NUM_MAKE_CORES}
extract-bc gawk
cd ..

mkdir obj-ubsan
cd obj-ubsan
CC=wllvm CFLAGS="-g -O1 -Xclang -disable-llvm-passes -D__NO_STRING_INLINES -D_FORTIFY_SOURCE=0 -U__OPTIMIZE__ -fsanitize=signed-integer-overflow -fsanitize=unsigned-integer-overflow -fsanitize=shift -fsanitize=bounds -fsanitize=pointer-overflow -fsanitize=null" ../configure --disable-mpfr --disable-largefile --disable-nls
make -j${NUM_MAKE_CORES}
extract-bc gawk
cd ..

mkdir obj-gcov
cd obj-gcov
CFLAGS="-g -fprofile-arcs -ftest-coverage -g -O0" ../configure --disable-mpfr --disable-largefile --disable-nls
make -j${NUM_MAKE_CORES}
cd ../..

########################## patch ##########################
wget https://ftp.gnu.org/gnu/patch/patch-2.7.6.tar.xz
tar -xf patch-2.7.6.tar.xz
cd patch-2.7.6

mkdir obj-llvm
cd obj-llvm
CC=wllvm CFLAGS="-g -O1 -Xclang -disable-llvm-passes -D__NO_STRING_INLINES -D_FORTIFY_SOURCE=0 -U__OPTIMIZE__ -fsanitize=signed-integer-overflow -fsanitize=unsigned-integer-overflow -fsanitize=shift -fsanitize=bounds -fsanitize=pointer-overflow -fsanitize=null" ../configure --disable-largefile
make -j${NUM_MAKE_CORES}
cd src
extract-bc patch
cd ../..

mkdir obj-ubsan
cd obj-ubsan
CC=wllvm CFLAGS="-g -O1 -Xclang -disable-llvm-passes -D__NO_STRING_INLINES -D_FORTIFY_SOURCE=0 -U__OPTIMIZE__" ../configure --disable-largefile
make -j${NUM_MAKE_CORES}
cd src
extract-bc patch
cd ../..

mkdir obj-gcov
cd obj-gcov
CFLAGS="-g -fprofile-arcs -ftest-coverage -g -O0" ../configure --disable-largefile
make -j${NUM_MAKE_CORES}
cd ../..

########################## binutils (objcopy and readelf) ##########################
wget https://ftp.gnu.org/gnu/binutils/binutils-2.36.tar.xz
tar -xf binutils-2.36.tar.xz
cd binutils-2.36

mkdir obj-llvm
cd obj-llvm
CC=wllvm CFLAGS="-g -O1 -Xclang -disable-llvm-passes -D__NO_STRING_INLINES -D_FORTIFY_SOURCE=0 -U__OPTIMIZE__" ../configure --disable-nls --disable-largefile --disable-gdb --disable-sim --disable-readline --disable-libdecnumber --disable-libquadmath --disable-libstdcxx --disable-ld --disable-gprof --disable-gas --disable-intl --disable-etc
make -j${NUM_MAKE_CORES}
cd binutils
find . -executable -type f | xargs -I '{}' extract-bc '{}'
cd ../..

mkdir obj-ubsan
cd obj-ubsan
CC=wllvm CFLAGS="-g -O1 -Xclang -disable-llvm-passes -D__NO_STRING_INLINES -D_FORTIFY_SOURCE=0 -U__OPTIMIZE__ -fsanitize=signed-integer-overflow -fsanitize=unsigned-integer-overflow -fsanitize=shift -fsanitize=bounds -fsanitize=pointer-overflow -fsanitize=null" ../configure --disable-nls --disable-largefile --disable-gdb --disable-sim --disable-readline --disable-libdecnumber --disable-libquadmath --disable-libstdcxx --disable-ld --disable-gprof --disable-gas --disable-intl --disable-etc
make -j${NUM_MAKE_CORES}
cd binutils
find . -executable -type f | xargs -I '{}' extract-bc '{}'
cd ../..

mkdir obj-gcov-objcopy obj-gcov-readelf
cd obj-gcov-objcopy
CFLAGS="-g -fprofile-arcs -ftest-coverage -g -O0" ../configure --disable-nls --disable-largefile --disable-gdb --disable-sim --disable-readline --disable-libdecnumber --disable-libquadmath --disable-libstdcxx --disable-ld --disable-gprof --disable-gas --disable-intl --disable-etc
make -j${NUM_MAKE_CORES}
cd ../obj-gcov-readelf
CFLAGS="-g -fprofile-arcs -ftest-coverage -g -O0" ../configure --disable-nls --disable-largefile --disable-gdb --disable-sim --disable-readline --disable-libdecnumber --disable-libquadmath --disable-libstdcxx --disable-ld --disable-gprof --disable-gas --disable-intl --disable-etc
make -j${NUM_MAKE_CORES}
cd ../..

########################## make ##########################
wget https://ftp.gnu.org/gnu/make/make-4.3.tar.gz
tar -xf make-4.3.tar.gz
cd make-4.3

mkdir obj-llvm
cd obj-llvm
CC=wllvm CFLAGS="-g -O1 -Xclang -disable-llvm-passes -D__NO_STRING_INLINES -D_FORTIFY_SOURCE=0 -U__OPTIMIZE__" ../configure --disable-nls --disable-largefile --disable-job-server --disable-load
make -j${NUM_MAKE_CORES}
extract-bc make
cd ..

mkdir obj-ubsan
cd obj-ubsan
CC=wllvm CFLAGS="-g -O1 -Xclang -disable-llvm-passes -D__NO_STRING_INLINES -D_FORTIFY_SOURCE=0 -U__OPTIMIZE__ -fsanitize=signed-integer-overflow -fsanitize=unsigned-integer-overflow -fsanitize=shift -fsanitize=bounds -fsanitize=pointer-overflow -fsanitize=null" ../configure --disable-nls --disable-largefile --disable-job-server --disable-load
make -j${NUM_MAKE_CORES}
extract-bc make
cd ..

mkdir obj-gcov
cd obj-gcov
CFLAGS="-g -fprofile-arcs -ftest-coverage -g -O0" ../configure --disable-nls --disable-largefile --disable-job-server --disable-load
make -j${NUM_MAKE_CORES}
cd ../..

########################## cjson ##########################
wget https://github.com/DaveGamble/cJSON/archive/refs/tags/v1.7.14.tar.gz
tar -xf v1.7.14.tar.gz
cd cJSON-1.7.14/
cp ${SOURCE_DIR}/benchmarks/cjson-test.c test.c

mkdir obj-llvm
cd obj-llvm
cmake .. -DENABLE_CJSON_TEST=On -DENABLE_CJSON_UTILS=On -DENABLE_CUSTOM_COMPILER_FLAGS=On -DENABLE_VALGRIND=Off -DENABLE_SANITIZERS=Off -DENABLE_SAFE_STACK=Off -DBUILD_SHARED_LIBS=Off -DCMAKE_C_COMPILER=wllvm -DCMAKE_C_FLAGS="-g -O1 -Xclang -disable-llvm-passes -D__NO_STRING_INLINES -D_FORTIFY_SOURCE=0 -U__OPTIMIZE__"
make -j${NUM_MAKE_CORES}
extract-bc cJSON_test
cd ..

mkdir obj-ubsan
cd obj-ubsan
cmake .. -DENABLE_CJSON_TEST=On -DENABLE_CJSON_UTILS=On -DENABLE_CUSTOM_COMPILER_FLAGS=On -DENABLE_VALGRIND=Off -DENABLE_SANITIZERS=Off -DENABLE_SAFE_STACK=Off -DBUILD_SHARED_LIBS=Off -DCMAKE_C_COMPILER=wllvm -DCMAKE_C_FLAGS="-g -O1 -Xclang -disable-llvm-passes -D__NO_STRING_INLINES -D_FORTIFY_SOURCE=0 -U__OPTIMIZE__ -fsanitize=signed-integer-overflow -fsanitize=unsigned-integer-overflow -fsanitize=shift -fsanitize=bounds -fsanitize=pointer-overflow -fsanitize=null"
make -j${NUM_MAKE_CORES}
extract-bc cJSON_test
cd ..

mkdir obj-gcov
cd obj-gcov
cmake .. -DENABLE_CJSON_TEST=On -DENABLE_CJSON_UTILS=On -DENABLE_CUSTOM_COMPILER_FLAGS=On -DENABLE_VALGRIND=Off -DENABLE_SANITIZERS=Off -DENABLE_SAFE_STACK=Off -DBUILD_SHARED_LIBS=Off -DCMAKE_C_FLAGS="-g -fprofile-arcs -ftest-coverage -g -O0"
make -j${NUM_MAKE_CORES}
cd ../..

########################## sqlite ##########################
mkdir sqlite
cd sqlite
wget https://www.sqlite.org/2020/sqlite-amalgamation-3330000.zip
unzip sqlite-amalgamation-3330000.zip

cp -r sqlite-amalgamation-3330000 sqlite-amalgamation-3330000-llvm
cd sqlite-amalgamation-3330000-llvm
wllvm -g -O1 -Xclang -disable-llvm-passes -D__NO_STRING_INLINES -D_FORTIFY_SOURCE=0 -U__OPTIMIZE__ -DSQLITE_THREADSAFE=0 -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_DEFAULT_MEMSTATUS=0 -DSQLITE_MAX_EXPR_DEPTH=0 -DSQLITE_OMIT_DECLTYPE -DSQLITE_OMIT_DEPRECATED -DSQLITE_DEFAULT_PAGE_SIZE=512 -DSQLITE_DEFAULT_CACHE_SIZE=10 -DSQLITE_DISABLE_INTRINSIC -DSQLITE_DISABLE_LFS -DYYSTACKDEPTH=20 -DSQLITE_OMIT_LOOKASIDE -DSQLITE_OMIT_WAL -DSQLITE_OMIT_PROGRESS_CALLBACK -DSQLITE_DEFAULT_LOOKASIDE='64,5' -DSQLITE_OMIT_PROGRESS_CALLBACK -DSQLITE_OMIT_SHARED_CACHE -I. shell.c sqlite3.c -o sqlite3
find . -executable -type f | xargs -I '{}' extract-bc '{}'
cd ..

cp -r sqlite-amalgamation-3330000 sqlite-amalgamation-3330000-ubsan
cd sqlite-amalgamation-3330000-ubsan
wllvm -g -O1 -Xclang -disable-llvm-passes -D__NO_STRING_INLINES -D_FORTIFY_SOURCE=0 -U__OPTIMIZE__ -fsanitize=signed-integer-overflow -fsanitize=unsigned-integer-overflow -fsanitize=shift -fsanitize=bounds -fsanitize=pointer-overflow -fsanitize=null -DSQLITE_THREADSAFE=0 -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_DEFAULT_MEMSTATUS=0 -DSQLITE_MAX_EXPR_DEPTH=0 -DSQLITE_OMIT_DECLTYPE -DSQLITE_OMIT_DEPRECATED -DSQLITE_DEFAULT_PAGE_SIZE=512 -DSQLITE_DEFAULT_CACHE_SIZE=10 -DSQLITE_DISABLE_INTRINSIC -DSQLITE_DISABLE_LFS -DYYSTACKDEPTH=20 -DSQLITE_OMIT_LOOKASIDE -DSQLITE_OMIT_WAL -DSQLITE_OMIT_PROGRESS_CALLBACK -DSQLITE_DEFAULT_LOOKASIDE='64,5' -DSQLITE_OMIT_PROGRESS_CALLBACK -DSQLITE_OMIT_SHARED_CACHE -I. shell.c sqlite3.c -o sqlite3
find . -executable -type f | xargs -I '{}' extract-bc '{}'
cd ..

cp -r sqlite-amalgamation-3330000 sqlite-amalgamation-3330000-gcov
cd sqlite-amalgamation-3330000-gcov
gcc -g -fprofile-arcs -ftest-coverage -O0 -DSQLITE_THREADSAFE=0 -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_DEFAULT_MEMSTATUS=0 -DSQLITE_MAX_EXPR_DEPTH=0 -DSQLITE_OMIT_DECLTYPE -DSQLITE_OMIT_DEPRECATED -DSQLITE_DEFAULT_PAGE_SIZE=512 -DSQLITE_DEFAULT_CACHE_SIZE=10 -DSQLITE_DISABLE_INTRINSIC -DSQLITE_DISABLE_LFS -DYYSTACKDEPTH=20 -DSQLITE_OMIT_LOOKASIDE -DSQLITE_OMIT_WAL -DSQLITE_OMIT_PROGRESS_CALLBACK -DSQLITE_DEFAULT_LOOKASIDE='64,5' -DSQLITE_OMIT_PROGRESS_CALLBACK -DSQLITE_OMIT_SHARED_CACHE -I. shell.c sqlite3.c -o sqlite3
cd ../..