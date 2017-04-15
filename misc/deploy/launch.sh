#!/bin/bash

GAME=game
DIR=`dirname "$0"`
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:"${DIR}"

cd "${DIR}"
./game $*
exit $?

