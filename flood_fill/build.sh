#! /usr/bin/env bash

set -xe

clang -S -emit-llvm -I../utils/raylib-5.5_linux_amd64/include/ test.c  -L../utils/raylib-5.5_linux_amd64/lib/ -l:libraylib.a -lm
clang -ggdb -o main main.ll -L../utils/raylib-5.5_linux_amd64/lib/ -l:libraylib.a -lm
