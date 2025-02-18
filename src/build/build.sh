#!/bin/bash

set -e

usage() {
    echo "Usage:"
    echo "$0 <build|install <destdir>>"
    exit 1
}

NAME="bls"

if [ "$#" -eq 0 ]; then
    usage "$1"
elif [ "$1" = "install" ] && [ "$#" = 1 ]; then
    usage "$1"
elif [ "$1" = "build" ]; then
    rm -r ./out
    cc -I../include -O2 -o "$NAME" ../*.c && mkdir out
    mv ./"$NAME" ./out
else
    sudo cp "./out/$NAME" "$2"
fi
