#! /usr/bin/env bash

set -e

clang -Wall -Wextra -pedantic -Wno-tautological-pointer-compare -o llvmgen llvmgen.c && ./llvmgen hello.bf > hello_gen.ll && clang -o hello hello_gen.ll
clang++ -Wall -Wextra $(llvm-config --cppflags) -pedantic -o main main.cpp $(llvm-config --ldflags --libs core) && ./main ./hello.bf -o hello_lib && clang++ -o hello hello_lib.o
