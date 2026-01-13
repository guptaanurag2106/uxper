#include <string.h>

#include <cassert>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <vector>

using namespace std;

void usage(const char *prog_name) {
    printf("cps — File compression utility\n\n");
    printf("Usage:\n");
    printf("  %s [OPTIONS] <INPUT>...\n\n", prog_name);
    printf("Arguments:\n");
    printf("  <INPUT>...     Files to compress\n\n");
    printf("Options:\n");
    printf("  -c, --compress       Compress input files\n");
    printf("  -e, --extract        Extract input files\n");
    printf("  -o, --output <FILE>  Output file name\n");
    // printf("  -l, --level <LEVEL>  Compression level (1-9, default: 6)\n");
    // printf("                       Only valid with -c/--compress\n");
    printf("  -h, --help           Print this help information\n");
    printf("  -v, --version        Print version information\n\n");
    printf("Examples:\n");
    printf("  %s -c test.txt -o test.cps  # Compress test.txt to test.cps\n",
           prog_name);
    // printf("  %s -c test.txt -l 3         # Compress test.txt at level 3\n",
    // prog_name);
    printf("  %s -e test.cps              # Extract test.cps\n", prog_name);
}

struct Bitops {
    ifstream &in;
    ofstream &out;
    unsigned char buffer;
    int bitCount = 0;

    void writeBit(bool bit) {
        buffer = (buffer << 1) | (bit ? 1 : 0);
        bitCount++;
        if (bitCount == 8) {
            out.write(reinterpret_cast<char *>(&buffer), 1);
            buffer = 0;
            bitCount = 0;
        }
    }

    void flush() {
        if (bitCount > 0) {
            buffer <<= (8 - bitCount);
            out.write(reinterpret_cast<char *>(&buffer), 1);
        }
        buffer = 0;
        bitCount = 0;
    }

    void writeByte(unsigned char buf) {
        if (bitCount == 0) {
            out.write(reinterpret_cast<char *>(&buf), 1);
        } else {
            for (int bit = 7; bit >= 0; --bit) {
                writeBit((buf >> bit) & 0x1);
            }
        }
    }

    unsigned char readNBits(unsigned char buf, int s, int e) {
        assert((e >= s) && (e - s) < 8 && "readNBits reads max 1 byte");
        unsigned char res = 0;

        for (int i = e; i >= s; i--) {
            res |= (buf & (1 << i)) >> s;
        }

        return res;
    }
};

constexpr int MAX_COUNT_LEN = 3;

bool compress_file(ifstream &inFile, ofstream &oFile) {
    Bitops bo = {.in = inFile, .out = oFile, .buffer = 0, .bitCount = 0};
    bool curr_bit;
    int curr_count = 0;

    vector<char> buffer(100);
    while (inFile.read(buffer.data(), buffer.size()) || inFile.gcount() > 0) {
        streamsize bytesRead = inFile.gcount();
        for (int i = 0; i < bytesRead; i++) {
            unsigned char byte = buffer[i];
            for (int bit = 7; bit >= 0; --bit) {
                bool val = (byte >> bit) & 0x1;
                if (curr_count == 0) {
                    curr_bit = val;
                    curr_count = 1;
                } else {
                    if (curr_bit == val) {
                        curr_count++;
                        if (curr_count == ((1 << MAX_COUNT_LEN) - 1)) {
                            curr_count--;
                            for (int j = MAX_COUNT_LEN - 1; j >= 0; --j) {
                                bo.writeBit((curr_count >> j) & 0x1);
                            }
                            bo.writeBit(curr_bit);
                            curr_count = 0;
                        }
                    } else {
                        curr_count--;
                        for (int j = MAX_COUNT_LEN - 1; j >= 0; --j) {
                            bo.writeBit((curr_count >> j) & 0x1);
                        }
                        bo.writeBit(curr_bit);
                        curr_bit = val;
                        curr_count = 1;
                    }
                }
            }
        }
    }

    bo.flush();

    return true;
}

bool extract_file(ifstream &inFile, ofstream &oFile) {
    Bitops bo = {.in = inFile, .out = oFile, .buffer = 0, .bitCount = 0};

    vector<char> buffer(100);
    while (inFile.read(buffer.data(), buffer.size()) || inFile.gcount() > 0) {
        streamsize bytesRead = inFile.gcount();
        for (int i = 0; i < bytesRead; i++) {
            unsigned char byte = buffer[i];
            int count1 = bo.readNBits(byte, 5, 7) + 1;
            bool bit1 = bo.readNBits(byte, 4, 4);
            int count2 = bo.readNBits(byte, 1, 3) + 1;
            bool bit2 = bo.readNBits(byte, 0, 0);

            for (int i = 0; i < count1; i++) {
                bo.writeBit(bit1);
            }
            for (int i = 0; i < count2; i++) {
                bo.writeBit(bit2);
            }
            // printf("%08b %d->%d %d->%d\n", byte, count1, bit1, count2, bit2);
        }
        // exit(0);
    }
    bo.flush();

    return true;
}

int main(int argc, char *argv[]) {
    const char *prog_name = argv[0];
    if (argc == 1) {
        usage(prog_name);
        return 0;
    }

    string input_file;
    string output_file;
    bool compress;  // true for compress, false for extract

    for (int i = 1; i < argc; i++) {
        if (string_view(argv[i]) == "-h" || string_view(argv[i]) == "--help") {
            usage(prog_name);
            return 0;
        }
        if (string_view(argv[i]) == "-c")
            compress = true;
        else if (string_view(argv[i]) == "-d")
            compress = false;
        else if (string_view(argv[i]) == "-o") {
            if (i == argc - 1) {
                fprintf(stderr, "missing output file name after '-o'\n");
                fprintf(stderr, "For more information, try %s -h \n",
                        prog_name);
                return 1;
            }
            output_file = argv[++i];
        } else {
            input_file = argv[i];
        }
    }
    if (input_file.empty()) {
        fprintf(stderr, "missing input file name\n");
        fprintf(stderr, "For more information, try %s -h \n", prog_name);
        return 1;
    }

    if (output_file.empty()) {
        size_t lastd = input_file.rfind('.');
        if (compress) {
            if (lastd == string::npos || lastd > input_file.size()) {
                output_file = input_file + ".cps";
            } else {
                output_file = input_file.substr(0, lastd) + ".cps";
            }
        } else {
            if (lastd == string::npos || lastd > input_file.size()) {
                output_file = input_file + ".txt";
            } else {
                output_file = input_file.substr(0, lastd) + ".txt";
            }
        }
    }
    printf("%s %s to %s\n", compress ? "Compressing" : "Extracting",
           input_file.c_str(), output_file.c_str());

    ifstream inFile;
    inFile.open(input_file, ios::in | ios::binary);
    if (!inFile.is_open()) {
        fprintf(stderr, "Cannot open input file %s: %s\n", input_file.c_str(),
                strerror(errno));
        return 1;
    }

    ofstream oFile;
    oFile.open(output_file);
    if (!oFile.is_open()) {
        fprintf(stderr, "Cannot open output file %s: %s\n", output_file.c_str(),
                strerror(errno));
        return 1;
    }

    if (compress) {
        if (!compress_file(inFile, oFile)) {
            fprintf(stderr, "Could not compress file\n");
        }
    } else {
        if (!extract_file(inFile, oFile)) {
            fprintf(stderr, "Could not extract file\n");
        }
    }
    inFile.close();
    oFile.close();

    return 0;
}
