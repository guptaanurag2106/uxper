#include <assert.h>
#include <bits/pthreadtypes.h>
#include <float.h>
#include <libdeflate.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

void usage(char *prog) { printf("Usage: %s <train.csv> <test.csv>\n", prog); }

typedef enum { DEFLATE, ZLIB, GZIP } Compression;

size_t get_compressed_length(const char *in, Compression compression,
                             struct libdeflate_compressor *compressor) {
    size_t outsize = strlen(in) * 2;
    void *test_out = malloc(outsize);

    if (test_out == NULL) {
        fprintf(stderr,
                "ERROR: `get_compressed_length` could not malloc out buffer\n");
        exit(1);
    }

    size_t compressed_size = libdeflate_deflate_compress(
        compressor, in, strlen(in), test_out, outsize);
    free(test_out);
    return compressed_size;
}

#define MAX_TITLE_LENGTH 256
#define MAX_DESC_LENGTH 1024
#define MAX_LINE_LENGTH (sizeof(int) + MAX_TITLE_LENGTH + MAX_DESC_LENGTH)
#define THREAD_COUNT 1
#define K_KNN 20

typedef struct {
    int id;
    char title[MAX_TITLE_LENGTH];
    char description[MAX_DESC_LENGTH];
} Article;

typedef struct {
    float ncd;
    uint article_id;
} Predict;

void parseLine(char *line, Article *article) {
    article->id = 0;
    article->description[0] = '\0';
    article->title[0] = '\0';

    char id[3] = "\0\0";  // %2 will read 2 char then 1 null pointer
    if ((sscanf(line, "%2[^,],%[^,],%[^\n]", id, article->title,
                article->description) == 3) ||
        (sscanf(line, "%2[^,],\"%[^\"]\",%[^\n]", id, article->title,
                article->description) == 3) ||
        (sscanf(line, "%2[^,],%[^,],\"%[^\"]\"", id, article->title,
                article->description) == 3) ||
        (sscanf(line, "%2[^,],\"%[^\"]\",\"%[^\"]\"", id, article->title,
                article->description) == 3)) {
        article->id = atoi(id);
    } else {
        fprintf(stderr, "ERROR: Could not parse line %s", line);
        article->id = -1;
    }
}

typedef struct {
    char *trainf;
    uint32_t *train_Clength;
    struct libdeflate_compressor *compressor;
    uint32_t c_test;
    Predict *test_ncd;
    char *art_d;
    uint32_t start;
    uint32_t end;
} NCDParams;

void *getNCD(void *params1) {
    NCDParams *params = (NCDParams *)params1;
    char *trainfile = params->trainf;
    uint32_t *train_Clength = params->train_Clength;
    struct libdeflate_compressor *compressor = params->compressor;
    uint32_t c_test = params->c_test;
    Predict *test_ncd = params->test_ncd;
    char *art_d = params->art_d;
    uint32_t start = params->start;
    uint32_t end = params->end;

    FILE *trainf = fopen(trainfile, "rb");
    if (trainf == NULL) {
        fprintf(stderr, "ERROR: Cannot open file %s\n", trainfile);
        exit(1);
    }

    size_t i = 0;
    Article tarticle;
    char tline[MAX_LINE_LENGTH];

    while (fgets(tline, sizeof(tline), trainf)) {
        if (i == 0 || i < start) {
            i++;
            continue;  // First line is table header
        }
        if (i >= end) break;
        parseLine(tline, &tarticle);
        if (tarticle.id == -1) {
            test_ncd[i - 1] = (Predict){.ncd = FLT_MAX};
            continue;
        }
        uint32_t c_train = train_Clength[i - 1];

        char *combined = NULL;
        combine_charp(art_d, tarticle.description, &combined);
        if (combined == NULL) {
            fprintf(stderr, "WARN: Could not combine strings for iteration");
            break;
        }
        uint32_t c_train_test =
            get_compressed_length(combined, DEFLATE, compressor);
        free(combined);

        float ncd =
            (float)(c_train_test - MIN(c_test, c_train)) / MAX(c_test, c_train);
        test_ncd[i - 1] = (Predict){.ncd = ncd, .article_id = tarticle.id};

        i++;
    }

    fclose(trainf);
    free(params);
    return NULL;
}

int compare(const void *a, const void *b) {
    return ((Predict *)a)->ncd > ((Predict *)b)->ncd;
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

    uint correct_predict = 0, predict = 0;

    size_t line_count = 0;
    while (fgets(tline, sizeof(tline), trainf)) {
        line_count++;
    }
    line_count--;  // ignore first header row
    if (fseek(trainf, 0, SEEK_SET) != 0) {
        perror("Error setting file position");
        fclose(trainf);
        fclose(testf);
        return -1;
    }
    Article article;
    Article tarticle;
    uint *train_Clength = malloc(line_count * sizeof(uint));
    if (train_Clength == NULL) {
        fprintf(stderr, "ERROR: Cannot allocate train_Clength\n");
        exit(1);
    }

    int max_id = -1;
    size_t i = 0;
    while (fgets(tline, sizeof(tline), trainf)) {
        if (i == 0) {
            i++;
            continue;  // First line is table header
        }
        parseLine(tline, &tarticle);
        if (tarticle.id == -1) {
            continue;
        }
        train_Clength[i - 1] = get_compressed_length(
            tarticle.description, DEFLATE, compressor);  // C(train)
        if (tarticle.id > max_id) max_id = tarticle.id;
        i++;
    }
    fclose(trainf);
    Predict *test_ncd = malloc(line_count * sizeof(Predict));
    if (test_ncd == NULL) {
        fprintf(stderr, "ERROR: Cannot allocate test_ncd\n");
        exit(1);
    }

    pthread_t threads[THREAD_COUNT];
    size_t it = 0;
    int knn_count[max_id + 1];
    while (fgets(line, sizeof(line), testf)) {
        if (it == 0) {
            it++;
            continue;  // First line is table header
        }
        parseLine(line, &article);
        if (article.id == -1) {
            continue;
        }
        uint c_test = get_compressed_length(article.description, DEFLATE,
                                            compressor);  // C(test)

        int actual_test_id = article.id;
        int predicted_test_id = -1;

        for (int k = 0; k < THREAD_COUNT; k++) {
            NCDParams *params = malloc(sizeof(NCDParams));

            params->trainf = train_file;
            params->train_Clength = train_Clength;
            params->compressor = compressor;
            params->c_test = c_test;
            params->test_ncd = test_ncd;
            params->art_d = article.description;
            params->start = 1 + k * line_count / THREAD_COUNT;
            params->end = 1 + (k + 1) * line_count / THREAD_COUNT;
            if (pthread_create(&threads[k], NULL, getNCD, (void *)params) !=
                0) {
                fprintf(stderr, "Failed to create thread");
                free(params);
                fclose(testf);
                exit(1);
            }
            // getNCD((void *)params);
        }
        // getNCD(trainf, train_Clength, compressor, c_test, test_ncd,
        //        article.description, 1, line_count + 1);
        for (int k = 0; k < THREAD_COUNT; k++) {
            pthread_join(threads[k], NULL);
        }
        qsort(test_ncd, line_count, sizeof(test_ncd[0]), compare);
        for (int k = 0; k <= max_id; k++) {
            knn_count[k] = 0;
        }
        for (int k = 0; k < K_KNN; k++) {
            knn_count[test_ncd[k].article_id]++;
        }
        int max_occur = -1;
        for (int k = 0; k <= max_id; k++) {
            if (knn_count[k] > max_occur) {
                predicted_test_id = k;
                max_occur = knn_count[k];
            }
        }

        printf("Iteration: %zu: Actual %d, Predicted %d \n", it, actual_test_id,
               predicted_test_id);
        predict++;
        correct_predict += actual_test_id == predicted_test_id;
        it++;
        if (it == 100) break;
    }

    printf("Accuracy: %.2f%% at K=%d\n", (float)correct_predict * 100 / predict, K_KNN);

    fclose(testf);
    return 0;
}
