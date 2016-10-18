#!/bin/bash

args="$@"
EXTRAFLAGS="-std=c99"

if [ ${#args} -gt 1 ] 
then
    case $1 in
        debug)
            EXTRAFLAGS="-std=c99 -g" ;;
        *)
            ;;
    esac
fi

cc $EXTRAFLAGS -o carml carml.c -L ~/homebrew/lib/ -I ~/homebrew/include/ -lgc
