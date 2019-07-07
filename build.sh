#!/bin/bash

args="$@"
EXTRAFLAGS="-std=c99"
LIBLOC="$HOME/homebrew/lib"
INCLOC="$HOME/homebrew/include"

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

if [ ${#args} -gt 1 ] 
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
        *)
            ;;
    esac
fi

echo "building carML/c carmlc.c"
cc $EXTRAFLAGS -o carmlc ./src/carmlc.c -L $LIBLOC -I $INCLOC -lgc && echo "[build success]" || echo "[build failed]"
