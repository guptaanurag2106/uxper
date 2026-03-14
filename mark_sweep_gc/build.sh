#! /usr/bin/env bash

set -xe
# gcc -ggdb -Wall -Wextra -Wpedantic -pthread -o main heap.c main.c
gcc -ggdb -Wall -Wextra -Wpedantic -DMEM_DEBUG -pthread -o main heap.c main.c
