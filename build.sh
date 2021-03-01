#!/bin/bash

args="$@"
EXTRAFLAGS="-std=c99"
LIBLOC="$HOME/homebrew/lib"
INCLOC="$HOME/homebrew/include"
BINLOC=""

if [ ! -d "$LIBLOC" ]; then
    LIBLOC="/usr/local/lib"
fi

if [ ! -d "$INCLOC" ]; then
    INCLOC="/usr/local/include"
fi

os=`uname`

case $os in
    Linux)
        # this is to stop GCC from complaining that
        # strlen isn't defined in string.h on my
        # chromebook.
        EXTRAFLAGS="-std=gnu99"
        ;;
    *)
        ;;
esac

if [ $# -gt 0 ]
then
    case $1 in
        softdebug)
            # this mode enables symbols, but does
            # not enable debug print, to cut down
            # on noise
            echo "[soft debugging enabled]"
            EXTRAFLAGS="-std=c99 -g" ;;
        debug)
            echo "[debugging enabled]"
            EXTRAFLAGS="-std=c99 -g -DDEBUG" ;;
        strict)
            echo "[strict mode]"
            EXTRAFLAGS="-std=c99 -Wall -Werror"
            ;;
        install)
            echo "[installing]"
            if [ $# -gt 1 ]
            then
                BINLOC=$2
            else
                BINLOC="$HOME/opt/bin"
            fi
            ;;
        *)
            ;;
    esac
fi

echo "building carML/c carmlc.c"
cc $EXTRAFLAGS -o carmlc ./src/carmlc.c ./src/self_tco.c -L $LIBLOC -I $INCLOC -I ./src -lgc && echo "[build success]" || echo "[build failed]"


if [ -d "$BINLOC" ]
then
    echo "installing to" $BINLOC
    strip carmlc
    cp carmlc $BINLOC
fi
