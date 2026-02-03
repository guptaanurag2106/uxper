#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void usage(const char *prog_name) {
    printf(
        "Usage: %s --encrypt/--decrypt -k <keyfile> -i <inputfile> [-o "
        "<outputfile>]\n\n",
        prog_name);
    printf("Options: \n");
    printf("Required Arguments: \n");
    printf(
        "  --encrypt          To encrypt <inputfile> with <keyfile> "
        "(default)\n");
    printf("  --decrypt          To decrypt <inputfile> with <keyfile>\n");
    printf("  -k <keyfile>       Path to key file (required)\n");
    printf("  -i <inputfile>     Input file (required)\n\n");
    printf("Optional Arguments: \n");
    printf("  -o <outputfile>    Output file (optional; default: stdout)\n");
    printf("  -h, --help         Show this help message and exit\n\n");
}

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
    const char *prog_name = argv[0];

    if (argc == 1) {
        fprintf(stderr, "No flags found, run %s -h to see all options\n",
                prog_name);
        return 1;
    }

    bool to_encrypt = true;  // true for encrypt false for decrypt
    char *key_file = "";
    char *input_file = "";
    char *output_file = NULL;
    const uint32_t rounds = 2;

    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
            usage(prog_name);
            return 0;
        }
        if (strcmp(arg, "--encrypt") == 0) {
            to_encrypt = true;
        } else if (strcmp(arg, "--decrypt") == 0) {
            to_encrypt = false;
        } else if (strcmp(arg, "-k") == 0) {
            if (i == (argc - 1)) {
                fprintf(stderr, "Expected <keyfile>, found nothing\n");
                return 1;
            }
            key_file = argv[++i];
        } else if (strcmp(arg, "-i") == 0) {
            if (i == (argc - 1)) {
                fprintf(stderr, "Expected <inputfile>, found nothing\n");
                return 1;
            }
            input_file = argv[++i];
        } else if (strcmp(arg, "-o") == 0) {
            if (i == (argc - 1)) {
                fprintf(stderr, "Expected <outputfile>, found nothing\n");
                return 1;
            }
            output_file = argv[++i];
        } else {
            fprintf(stderr, "Unknown flag %s\n", arg);
            return 1;
        }
    }

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
