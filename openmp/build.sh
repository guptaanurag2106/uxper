#!/usr/bin/env bash

set -e

gcc -Wall -Wextra -Wpedantic -fopenmp -o main main.c -lm
gcc -Wall -Wextra -Wpedantic -fopenmp -o mandel mandel.c -lm
gcc -Wall -Wextra -Wpedantic -fopenmp -o linked linked.c -lm
gcc -Wall -Wextra -Wpedantic -fopenmp -o prod_cons prod_cons.c -lm
gcc -Wall -Wextra -Wpedantic -fopenmp -o pi_mc pi_mc.c random.c -lm
