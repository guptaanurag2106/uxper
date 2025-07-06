#include "utils.h"
#include <assert.h>
#include <float.h>
#include <libdeflate.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void usage(char *prog) { printf("Usage: %s <train.csv> <test.csv>\n", prog); }

typedef enum { DEFLATE, ZLIB, GZIP } Compression;

size_t get_compressed_length(const char *in, Compression compression, struct libdeflate_compressor *compressor) {
    size_t outsize = strlen(in) * 2;
    void *test_out = malloc(outsize);

    if (test_out == NULL) {
        fprintf(stderr, "ERROR: `get_compressed_length` could not malloc out buffer\n");
        exit(1);
    }

    size_t compressed_size = libdeflate_deflate_compress(compressor, in, strlen(in), test_out, outsize);
    free(test_out);
    return compressed_size;
}

#define MAX_TITLE_LENGTH 256
#define MAX_DESC_LENGTH 1024
#define MAX_LINE_LENGTH (sizeof(int) + MAX_TITLE_LENGTH + MAX_DESC_LENGTH)

typedef struct {
    int id;
    char title[MAX_TITLE_LENGTH];
    char description[MAX_DESC_LENGTH];
} Article;

void parseLine(char *line, Article *article) {
    article->id = 0;
    article->description[0] = '\0';
    article->title[0] = '\0';

    char id[3] = "\0\0"; // %2 will read 2 char then 1 null pointer
    if ((sscanf(line, "%2[^,],%[^,],%[^\n]", id, article->title, article->description) == 3) ||
        (sscanf(line, "%2[^,],\"%[^\"]\",%[^\n]", id, article->title, article->description) == 3) ||
        (sscanf(line, "%2[^,],%[^,],\"%[^\"]\"", id, article->title, article->description) == 3) ||
        (sscanf(line, "%2[^,],\"%[^\"]\",\"%[^\"]\"", id, article->title, article->description) == 3)) {
        article->id = atoi(id);
    } else {
        fprintf(stderr, "ERROR: Could not parse line %s", line);
        article->id = -1;
    }
}

int main(int argc, char **argv) {
    char *prog_name = shift(&argc, &argv);
    char *train_file = shift(&argc, &argv);
    if (train_file == NULL) {
        usage(prog_name);
        return 1;
    }
    char *test_file = shift(&argc, &argv);
    if (test_file == NULL) {
        usage(prog_name);
        return 1;
    }

    struct libdeflate_compressor *compressor = libdeflate_alloc_compressor(10);

    FILE *trainf = fopen(train_file, "rb");
    if (trainf == NULL) {
        fprintf(stderr, "ERROR: Cannot open file %s\n", train_file);
        exit(1);
    }
    FILE *testf = fopen(test_file, "rb");
    if (testf == NULL) {
        fprintf(stderr, "ERROR: Cannot open file %s\n", test_file);
        exit(1);
    }

    char tline[MAX_LINE_LENGTH];
    char line[MAX_LINE_LENGTH];

    size_t it = 0;

    uint correct_predict = 0, predict = 0;

    Ivector *train_Clength = init_Ivector(100);

    Article article;
    Article tarticle;
    while (fgets(line, sizeof(line), testf)) {
        it++;
        if (it == 1)
            continue; // First line is table header
        parseLine(line, &article);
        if (article.id == -1) {
            continue;
        }
        uint c_test = get_compressed_length(article.description, DEFLATE, compressor); // C(test)

        int actual_test_id = article.id;
        int predicted_test_id = -1;
        float ncd_min = FLT_MAX;

        size_t i = 0;
        while (fgets(tline, sizeof(tline), trainf)) {
            if (i == 0) {
                i++;
                continue; // First line is table header
            }
            parseLine(tline, &tarticle);
            if (tarticle.id == -1) {
                continue;
            }
            uint c_train;
            if (train_Clength->size >= i) {
                c_train = get_Ivector(train_Clength, i - 1);
            } else {
                c_train = get_compressed_length(tarticle.description, DEFLATE, compressor); // C(train)
                push_back_Ivector(train_Clength, c_train);
            }
            i++;

            char *combined = NULL;
            combine_charp(article.description, tarticle.description, &combined);
            if (combined == NULL) {
                fprintf(stderr, "WARN: Could not combine strings for iteration: %zu  ", it);
                break;
            }
            uint c_train_test = get_compressed_length(combined, DEFLATE, compressor);
            free(combined);

            float ncd = (float)(c_train_test - MIN(c_test, c_train)) / MAX(c_test, c_train);

            if (ncd < ncd_min) {
                ncd_min = ncd;
                predicted_test_id = tarticle.id;
            }
        }

        if (fseek(trainf, 0, SEEK_SET) != 0) {
            perror("Error setting file position");
            fclose(trainf);
            fclose(testf);
            return -1;
        }

        printf("Iteration: %zu: Actual %d, Predicted %d \n", it, actual_test_id, predicted_test_id);
        predict++;
        correct_predict += actual_test_id == predicted_test_id;
        if (it == 50)
            break;
    }

    printf("Accuracy: %.2f%%\n", (float)correct_predict * 100 / predict);

    fclose(trainf);
    fclose(testf);
    return 0;
}
