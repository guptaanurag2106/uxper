#!/usr/bin/env bash

set -xe

# xtea
head -c 16 /dev/urandom > xtea_key.bin
