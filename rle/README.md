# CPS: File Compression Utility

A custom file compression tool implementing Run-Length Encoding (RLE) bit-packing strategies.

## Context
- [Run-Length Encoding (RLE)](https://en.wikipedia.org/wiki/Run-length_encoding)

## Build & Run

```bash
# Build
g++ main.cpp -o cps

# Run
./cps -h                # Help
./cps -c input.txt      # Compress
./cps -d input.cps      # Decompress
```
