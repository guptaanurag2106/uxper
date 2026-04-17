#! /usr/bin/env bash

set -xe
# gcc -ggdb -Wall -Wextra -Wpedantic -pthread -o main heap.c main.c
# gcc -ggdb -Wall -Wextra -Wpedantic -DMEM_DEBUG -pthread -o main heap.c main.c
clang -ggdb -Wall -Wextra -DMEM_DEBUG -pthread -o main heap.c main.c
# F's up the GC
clang -ggdb -Wall -Wextra -DMEM_DEBUG -pthread -O0 -fno-omit-frame-pointer -fsanitize=address,undefined,leak -o main_asan heap.c main.c
