#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define FLAG_IMPLEMENTATION
#include "../utils/flag.h"

bool encrypt(uint32_t rounds, FILE *in, FILE *key, FILE *out) {
    uint32_t in_buf[2] = {0};
    uint32_t key_buf[4] = {0};

    size_t read = fread(key_buf, sizeof(key_buf), 1, key);

    if (read < 1) {  // read less than 1 item (item of size key_buf)
        fprintf(stderr, "Key must be 128 bits long\n");
        return false;
    }

    while ((read = fread(in_buf, sizeof(in_buf), 1, in)) > 0) {
        uint32_t v0 = in_buf[0], v1 = in_buf[1], sum = 0, delta = 0x9E3779B9;

        for (size_t i = 0; i < rounds; i++) {
            v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key_buf[sum & 3]);
            sum += delta;
            v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^
                  (sum + key_buf[(sum >> 11) & 3]);
        }
        fwrite(&v0, sizeof(v0), 1, out);
        fwrite(&v1, sizeof(v1), 1, out);
        in_buf[0] = 0;
        in_buf[1] = 1;
    }

    return true;
}

bool decrypt(uint32_t rounds, FILE *in, FILE *key, FILE *out) {
    uint32_t in_buf[2] = {0};
    uint32_t key_buf[4] = {0};

    size_t read = fread(key_buf, sizeof(key_buf), 1, key);

    if (read < 1) {  // read less than 1 item (item of size key_buf)
        fprintf(stderr, "Key must be 128 bits long\n");
        return false;
    }

    while ((read = fread(in_buf, sizeof(in_buf), 1, in)) > 0) {
        uint32_t v0 = in_buf[0], v1 = in_buf[1], delta = 0x9E3779B9,
                 sum = delta * rounds;
        for (size_t i = 0; i < rounds; i++) {
            v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^
                  (sum + key_buf[(sum >> 11) & 3]);
            sum -= delta;
            v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key_buf[sum & 3]);
        }
        fwrite(&v0, sizeof(v0), 1, out);
        fwrite(&v1, sizeof(v1), 1, out);
        in_buf[0] = 0;
        in_buf[1] = 1;
    }
    return true;
}

int main(int argc, char **argv) {
    bool *encrypt_mode = flag_bool("encrypt", false, "To encrypt <inputfile> with <keyfile> (default)");
    bool *decrypt_mode = flag_bool("decrypt", false, "To decrypt <inputfile> with <keyfile>");
    char **key_file_ptr = flag_string("key", "", "Path to key file", .required = true, .single_char = 'k');
    char **input_file_ptr = flag_string("input", "", "Input file", .required = true, .single_char = 'i');
    char **output_file_ptr = flag_string("output", "", "Output file (optional; default: stdout)", .single_char = 'o');

    flag_parse(argc, argv, "XTEA Cipher implementation");

    bool to_encrypt = true;
    if (*decrypt_mode) to_encrypt = false;
    (void)encrypt_mode; // Suppress unused warning

    const uint32_t rounds = 2;
    char *key_file = *key_file_ptr;
    char *input_file = *input_file_ptr;
    char *output_file = (*output_file_ptr && **output_file_ptr) ? *output_file_ptr : NULL;

    FILE *in = fopen(input_file, "rb");
    if (in == NULL) {
        fprintf(stderr, "Cannot open %s because %s\n", input_file,
                strerror(errno));
        return 1;
    }

    FILE *key = fopen(key_file, "rb");
    if (key == NULL) {
        fprintf(stderr, "Cannot open %s because %s\n", key_file,
                strerror(errno));
        return 1;
    }

    FILE *out;
    if (output_file) {
        out = fopen(output_file, "wb");
        if (out == NULL) {
            fprintf(stderr, "Cannot open %s because %s\n", output_file,
                    strerror(errno));
            return 1;
        }
    } else
        out = stdout;

    if (to_encrypt) {
        if (!encrypt(rounds, in, key, out)) return 1;
        printf("Successfully encrypted %s and written output to %s\n",
               input_file, output_file ? output_file : "stdout");
    } else {
        if (!decrypt(rounds, in, key, out)) return 1;
        printf("Successfully decrypted %s and written output to %s\n",
               input_file, output_file ? output_file : "stdout");
    }

    fclose(in);
    fclose(key);
    fclose(out);

    return 0;
}
