#!/bin/bash

args="$@"
EXTRAFLAGS=""

if [ ${#args} -gt 1 ] 
then
    case $1 in
        debug)
            EXTRAFLAGS="-g" ;;
        *)
            ;;
    esac
fi

cc $EXTRAFLAGS -o carml carml.c -L ~/homebrew/lib/ -I ~/homebrew/include/ -lgc
