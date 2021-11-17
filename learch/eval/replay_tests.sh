#!/bin/bash

TESTS_DIR=${1}
GCOV_PATH=${2}
GCOV_DIR=${3}

find ${TESTS_DIR} -type f -name *.gcno | while read f
do
    rm ${f}
done

find ${TESTS_DIR} -type f -name *.gcda | while read f
do
    rm ${f}
done

find ${GCOV_DIR} -type f -name *.gcda | while read f
do
    rm ${f}
done

${SOURCE_DIR}/replay.sh ${GCOV_PATH} ${TESTS_DIR}/*.ktest

find ${GCOV_DIR} -type f -name *.gcda | while read f
do
    mkdir -p ${TESTS_DIR}/$(dirname ${f})
    cp ${f} ${TESTS_DIR}/${f}
done

find ${GCOV_DIR} -type f -name *.gcno | while read f
do
    mkdir -p ${TESTS_DIR}/$(dirname ${f})
    cp ${f} ${TESTS_DIR}/${f}
done

rm -rf /tmp/klee-replay-*