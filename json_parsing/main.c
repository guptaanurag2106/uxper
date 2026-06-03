#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define JSON_IMPLEMENTATION
#define UTILS_IMPLEMENTATION
#define ARENA_IMPLEMENTATION
#include "json.h"

int main(int argc, char **argv) {
    Log_set_level(Log_Error);
    const char *prog_name = shift(&argc, &argv);
    if (argc == 0) {
        printf("Usage: %s <input_toml_file>\n", prog_name);
        fprintf(stderr, "No input file provided\n");
        exit(1);
    }

    char *input_file = shift(&argc, &argv);
    Json *json = json_parse_file(input_file);
    if (json == NULL) {
        fprintf(stderr, "%s\n", json_get_error());
        json_free(json);
        return 1;
    }

    Log(Log_Info, "dumping: %s", json->input_file);
    //json_dump(json, stdout, false);
    //json_dump(json, stdout, true);
    json_free(json);

    return 0;
}
