#!/bin/bash

PROG=${1}

PROG_NAME=$(basename ${PROG})
KLEE_REPLAY="klee-replay"

rm -rf ${SANDBOX_DIR}/sandbox-replay-${PROG_NAME}
mkdir -p ${SANDBOX_DIR}/sandbox-replay-${PROG_NAME}
tar xvf ${SOURCE_DIR}/sandbox.tgz -C ${SANDBOX_DIR}/sandbox-replay-${PROG_NAME} > /dev/null
cd ${SANDBOX_DIR}/sandbox-replay-${PROG_NAME}/sandbox

for tcase in "${@:2}"
do	
    env -i /bin/bash -c "(source ${SANDBOX_DIR}/testing-env.sh; timeout 30 klee-replay ${PROG} ${tcase} >& /dev/null)"
done
