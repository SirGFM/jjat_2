#!/bin/bash

GAME=game
DIR=`dirname "$0"`
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:"${DIR}"

usage() {
    echo 'Usage:'
    echo '    --single-player-mode | -1p: Launch the game in single player asynchronous mode'
    echo '    --two-players-mode | -2p: Launch the game in two players mode'
    echo '    --synchronous-single-player | -s1p: Launch the game in synchronous single player mode'
    echo '    --help | -h: Print this usage information'
    exit 0
}

# Ease launching with custom key bindings
if [ $# -eq 1 ]; then
    case $1 in
        --single-player-mode | -1p)
            ARGS=-k\ 'SL:6005200b01;SR:6105300d;SJ:5a00214;SA:5b00c;GL:64047;GR:65048;GJ:5d049;GA:5e04a;P:68042;SW:6904344;'
        ;;
        --two-players-mode | -2p)
            ARGS=-k\ 'SL:600520;SR:610530;SJ:560;SA:570580;GL:60152147;GR:61153148;GJ:56115;GA:57158116;P:68068142;SW:5905914344;'
        ;;
        --synchronous-single-player | -s1p)
            ARGS=-c\ -k\ 'SL:60052047;SR:61053048;SJ:56015;SA:57058016;P:68042;SW:5904344;'
        ;;
        --help | -h)
            usage
        ;;
        *)
            echo 'No option selected!'
            usage
        ;;
    esac
else
    ARGS=$*
fi

cd "${DIR}"
./game ${ARGS}
exit $?

