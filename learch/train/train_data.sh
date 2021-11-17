#!/bin/bash

PROG=${1}
OUTPUT_DIR=${2}
MAX_TIME=${3}
ITER=${4}
SEARCHER=${5}

LLVM_PATH=${SOURCE_DIR}/benchmarks/coreutils-8.31/obj-llvm/src/${PROG}.bc
GCOV_PATH=${SOURCE_DIR}/benchmarks/coreutils-8.31/obj-gcov-${PROG}/src/${PROG}
GCOV_DIR=${SOURCE_DIR}/benchmarks/coreutils-8.31/obj-gcov-${PROG}
items=(${SEARCHER//##/ })
searcher_name=${items[0]}
searcher_path=${items[1]}
if [[ "${searcher_name}" == "feedforward" ]]; then
    searcher_options="--search=ml --model-type=feedforward --model-path=${searcher_path}"
elif [[ "${searcher_name}" == "feedforward" ]]; then
    searcher_options="--search=ml --model-type=linear --model-path=${searcher_path}"
elif [[ "${searcher_name}" == "feedforward" ]]; then
    searcher_options="--search=ml --model-type=rnn --model-path=${searcher_path}"
else
    searcher_options="--search=${searcher_name}"
fi
mkdir -p ${OUTPUT_DIR}/${searcher_name}-${ITER}
rm -rf ${SANDBOX_DIR}/sandbox-${PROG}
mkdir -p ${SANDBOX_DIR}/sandbox-${PROG}
tar -xvf ${SOURCE_DIR}/sandbox.tgz -C ${SANDBOX_DIR}/sandbox-${PROG} > /dev/null

klee --simplify-sym-indices --write-cvcs --write-cov --output-module --disable-inlining \
--optimize --use-forked-solver --use-cex-cache --libc=uclibc --posix-runtime \
--external-calls=all --watchdog --max-memory-inhibit=false --switch-type=internal \
--dump-states-on-halt=false \
--output-dir=${OUTPUT_DIR}/${searcher_name}-${ITER}/${PROG} --env-file=${SANDBOX_DIR}/test.env --run-in-dir=${SANDBOX_DIR}/sandbox-${PROG}/sandbox \
--max-memory=4096 --max-time=${MAX_TIME}s \
--feature-dump --feature-extract --use-branching-search ${searcher_options} \
${LLVM_PATH} \
--sym-args 0 1 10 --sym-args 0 2 2 --sym-files 1 8 --sym-stdin 8 --sym-stdout

python3 score.py --prog ${GCOV_PATH} --src_dir ${GCOV_DIR} --tests_dir ${OUTPUT_DIR}/${searcher_name}-${ITER}/${PROG}