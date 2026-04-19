#!/bin/bash

ROOT="../"

cd "$ROOT" || exit 1

cd ./game-engine || exit 1
make
RESULT=$?

if [ $RESULT -ne 0 ]; then
    CODE=1
    echo "Error inside Makefile with exit code $RESULT."
    exit $CODE
fi

cd ../UI || exit 1
cargo run
RESULT=$?

if [ $RESULT -ne 0 ]; then
    CODE=1
    echo "Error inside Cargo Run with exit code $RESULT."
    exit $CODE
fi

exit 0
