#!/bin/bash

PROG=${1}
OUTPUT_DIR=${2}
UBSAN=${3}
MAX_TIME=${4}
SEARCHER=${5}

if [[ "${UBSAN}" == "yes" ]]; then
    BC_PATH=${SOURCE_DIR}/benchmarks/coreutils-8.31/obj-ubsan/src/${PROG}.bc
else
    BC_PATH=${SOURCE_DIR}/benchmarks/coreutils-8.31/obj-llvm/src/${PROG}.bc
fi
items=(${SEARCHER//##/ })
searcher_name=${items[0]}
searcher_path=${items[1]}
if [[ "${searcher_name}" == "feedforward" ]]; then
    searcher_options="--feature-extract --search=ml --model-type=feedforward --model-path=${searcher_path}"
elif [[ "${searcher_name}" == "linear" ]]; then
    searcher_options="--feature-extract --search=ml --model-type=linear --model-path=${searcher_path}"
elif [[ "${searcher_name}" == "rnn" ]]; then
    searcher_options="--feature-extract --search=ml --model-type=rnn --model-path=${searcher_path}"
else
    searcher_options="--search=${searcher_name}"
fi
mkdir -p ${OUTPUT_DIR}/${searcher_name}
rm -rf ${SANDBOX_DIR}/sandbox-${PROG}
mkdir -p ${SANDBOX_DIR}/sandbox-${PROG}
tar -xvf ${SOURCE_DIR}/sandbox.tgz -C ${SANDBOX_DIR}/sandbox-${PROG} > /dev/null

klee --simplify-sym-indices --write-cvcs --write-cov --output-module --disable-inlining \
--optimize --use-forked-solver --use-cex-cache --libc=uclibc --posix-runtime \
--external-calls=all --watchdog --max-memory-inhibit=false --switch-type=internal \
--only-output-states-covering-new \
--dump-states-on-halt=false \
--output-dir=${OUTPUT_DIR}/${searcher_name}/${PROG} --env-file=${SANDBOX_DIR}/test.env --run-in-dir=${SANDBOX_DIR}/sandbox-${PROG}/sandbox \
--max-memory=4096 --max-time=${MAX_TIME}s \
--use-branching-search ${searcher_options} \
${BC_PATH} \
--sym-args 0 1 10 --sym-args 0 2 2 --sym-files 1 8 --sym-stdin 8 --sym-stdout