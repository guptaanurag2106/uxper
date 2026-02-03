# XOR Cipher 

## Context
- [XTEA](https://en.wikipedia.org/wiki/XTEA)
- [XOR Cipher](https://en.wikipedia.org/wiki/XOR_cipher)

## Build & Run

```bash
# Get keys
./keygen.sh
# Build
make

# Run
# xtea
./xtea --encrypt -i ./in.txt -k ./xtea_key.bin -o ./xtea_out.bin 
./xtea --decrypt -i ./xtea_out.bin -k ./xtea_key.bin -o ./out.txt
```
