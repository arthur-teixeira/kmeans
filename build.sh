#!/bin/sh

set -xe

CFLAGS="-Wall -Wextra -ggdb -I./raylib/"
LIBS="-lraylib -lm"

mkdir -p ./bin/

clang $CFLAGS -o ./bin/kmeans  ./*.c $LIBS -L./bin/
