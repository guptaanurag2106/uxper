#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define JSON_IMPLEMENTATION
#define ARENA_IMPLEMENTATION
#include "json.h"

#define UTILS_IMPLEMENTATION
#include "utils.h"

static char *format_json_error(const Json_Error *err) {
    if (err == NULL) return NULL;
    return temp_sprintf("%s:%d:%d: %s", err->source, err->line, err->column,
                        err->message);
}

#define TEST_FOLDER "tests/"
#define EXPECTED_SUFFIX ".expected"
#define GOT_SUFFIX ".got"

const char *test_files[] = {
    TEST_FOLDER "fail-01-trailing-garbage.json",
    TEST_FOLDER "fail-02-missing-delimiters.json",
    TEST_FOLDER "fail-03-invalid-values.json",
    TEST_FOLDER "fail-04-extra-commas.json",
    TEST_FOLDER "fail-05-double-root.json",
    TEST_FOLDER "fail-06-string-newline.json",
    TEST_FOLDER "fail-07-big-number.json",
    TEST_FOLDER "pass-01-basic-values.json",
    TEST_FOLDER "pass-02-arrays-and-nesting.json",
    TEST_FOLDER "pass-03-escaped-strings.json",
    TEST_FOLDER "pass-04-numbers.json",
    TEST_FOLDER "pass-05-empty-structures.json",
    TEST_FOLDER "pass-06-top-level-array.json",
    TEST_FOLDER "pass-07-whitespace-handling.json",
    TEST_FOLDER "sample.json",
    TEST_FOLDER "test_getters.c",
    TEST_FOLDER "test_creation.c",
    TEST_FOLDER "test_embedded.c",
};

int run(const char *file_name, char **res) {
    // checking if ends with c then run as c file
    if (file_name[strlen(file_name) - 1] == 'c') {
        char *command = temp_sprintf(
            "cc -I. -I../utils/ -o test.out %s && stdbuf -o0 ./test.out > "
            "/tmp/test_res 2>&1",
            file_name);
        system(command);
        *res = read_entire_file("/tmp/test_res");
        if (*res == NULL) return -1;
        return 0;
    } else {
        char *file_content = read_entire_file(file_name);
        if (file_content == NULL) {
            return -1;
        }

        Json_Error err = {0};
        Json *json = json_parse_string(file_content, &err);
        if (json == NULL) {
            *res = format_json_error(&err);
            free(file_content);
            return 1;
        }

        Json_Error dump_err = {0};
        *res = json_stringify(json, true, &dump_err);
        if (*res == NULL) {
            *res = format_json_error(&dump_err);
            json_free(json);
            free(file_content);
            return 1;
        }
        json_free(json);
        free(file_content);
        return 0;
    }
    return -1;
}

int record(const char *filename) {
    int result = 0;
    char *res = NULL;
    int ret = run(filename, &res);
    if (ret == -1) {
        result = 1;
    }

    const char *out_file_name = temp_sprintf("%s%s", filename, EXPECTED_SUFFIX);

    FILE *f = fopen(out_file_name, "w");
    if (f == NULL) {
        Log(Log_Error, "Cannot open out file %s: %s", out_file_name,
            strerror(errno));
        result = 1;
    }

    // fprintf(f, "%d:%s", ret, res);
    fprintf(f, "%s", res);
    Log(Log_Info, "Recorded output of %s into %s", filename, out_file_name);
    fclose(f);
    if (ret == 0) free(res);
    return result;
}

int recordAll(void) {
    int result = 0;
    for (size_t i = 0; i < ARRAY_LENGTH(test_files); i++) {
        int res = record(test_files[i]);
        if (res == 1) result = 1;
    }
    return result;
}

int test(const char *filename) {
    int result = 0;
    char *res = NULL;
    int ret = run(filename, &res);
    if (ret == -1) {
        result = 1;
    }

    const char *out_file_name = temp_sprintf("%s%s", filename, EXPECTED_SUFFIX);

    char *out_file_content = read_entire_file(out_file_name);
    if (out_file_content == NULL) {
        Log(Log_Error, "Test failed for %s: Cannot read expected out file %s",
            filename, out_file_name);
        result = 1;
        goto end;
    }
    //
    // if (strlen(out_file_content) < 2 ||
    //     !(out_file_content[0] == '0' || out_file_content[0] == '1') ||
    //     out_file_content[1] != ':') {
    //     Log(Log_Error,
    //         "Test failed for %s: Incorrect format for expected out file
    //         %s", test_json_files[i], out_file_name);
    //     result = 1;
    //     goto end;
    // }
    //
    // if ((ret == 0 && out_file_content[0] != '0') ||
    //     (ret == 1 && out_file_content[0] != '1')) {
    //     Log(Log_Error, "Test failed for %s: Expected return code %c got
    //     %d",
    //         test_json_files[i], out_file_content[0], ret);
    //     result = 1;
    //     goto end;
    // }

    if (strlen(res) != strlen(out_file_content) ||
        strncmp(out_file_content, res, strlen(res)) != 0) {
        const char *got_file_name = temp_sprintf("%s%s", filename, GOT_SUFFIX);

        FILE *f = fopen(got_file_name, "w");
        if (f == NULL) {
            Log(Log_Error, "Cannot open received output file %s: %s",
                got_file_name, strerror(errno));
            result = 1;
            goto end;
        }

        fprintf(f, "%s", res);
        fclose(f);
        Log(Log_Error,
            "Test failed for %s: Mismatched output, run `diff %s %s` for "
            "checking the difference",
            filename, out_file_name, got_file_name);
        result = 1;
        goto end;
    }

    Log(Log_Info, "Test successfull for %s", filename);

end:
    if (ret == 0) free(res);
    free(out_file_content);

    return result;
}

int testAll(void) {
    int result = 0;
    for (size_t i = 0; i < ARRAY_LENGTH(test_files); i++) {
        int res = test(test_files[i]);
        if (res == 1) result = 1;
    }
    return result;
}

int main(int argc, char **argv) {
    const char *prog_name = shift(&argc, &argv);
    if (argc == 0) {
        printf("Usage: %s [record|test]\n", prog_name);
        fprintf(stderr, "No command provided\n");
        exit(1);
    }

    const char *command = shift(&argc, &argv);
    char *file_name = NULL;
    if (argc != 0) {
        file_name = shift(&argc, &argv);
    }
    if (strncmp(command, "record", 6) == 0) {
        Log(Log_Info, "Recording current behaviour");

        int result = -1;
        if (file_name) {
            result = record(file_name);
        } else {
            result = recordAll();
        }
        if (result == 0) {
            Log(Log_Info, "Recorded current behaviour successfully");
        } else {
            Log(Log_Error,
                "Error(s) recording current behaviour, please check logs");
            return 1;
        }
    } else if (strncmp(command, "test", 4) == 0) {
        Log(Log_Info, "Testing current behaviour");
        int result = -1;
        if (file_name) {
            result = test(file_name);
        } else {
            result = testAll();
        }
        if (result == 0) {
            Log(Log_Info, "Tested all files successfully");
        } else {
            Log(Log_Error,
                "Error(s) testing current behaviour, please check logs");
            return 1;
        }
    } else {
        printf("Usage: %s [record|test]\n", prog_name);
        fprintf(stderr, "Unknown command '%s' provided\n", command);
        return 1;
    }

    return 0;
}
