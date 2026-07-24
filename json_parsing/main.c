#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define JSON_IMPLEMENTATION
#define ARENA_IMPLEMENTATION
#include <cJSON.h>

#include "json.h"

#define UTILS_IMPLEMENTATION
#include "utils.h"

int main(int argc, char **argv) {
    Log_set_level(Log_Error);
    const char *prog_name = shift(&argc, &argv);
    if (argc == 0) {
        printf("Usage: %s <input_toml_file>\n", prog_name);
        fprintf(stderr, "No input file provided\n");
        exit(1);
    }

    char *input_file = shift(&argc, &argv);

    char *scene_file_content = read_entire_file(input_file);
    struct timeval start, end;
    gettimeofday(&start, NULL);
    json_Error err = {0};
    Json *json = json_parse_string(scene_file_content, &err);
    if (json == NULL) {
        printf("%s:%d:%d: %s\n", err.source, err.line, err.column, err.message);
    } else {
        json_dump(json, stdout, true, &err);
        gettimeofday(&end, NULL);
        json_free(json);
        printf("json.h time taken: %fms\n", timersub_ms(&end, &start));
    }

    gettimeofday(&start, NULL);
    cJSON *cjson = cJSON_Parse(scene_file_content);
    if (cjson == NULL) {
        const char *indicator =
            "                                                          ^";
        fprintf(stderr, "load_scene: JSON parse error near: %.30s\n%s",
                cJSON_GetErrorPtr() ? cJSON_GetErrorPtr() - 15 : "unknown",
                indicator);
        free(scene_file_content);
    } else {
        printf("%s\n", cJSON_PrintUnformatted(cjson));
        gettimeofday(&end, NULL);
        free(scene_file_content);
        printf("cJSON time taken: %fms\n", timersub_ms(&end, &start));
        cJSON_Delete(cjson);
    }

    return 0;
}
// TODO:in sample.json the float comes as 3.141593 and not as 3.141592653589793,
// and having "smallInt": -9223372036854775808 makes it fail with
// numbers.largeInt	9223372036854775807	9.2233720368547758e+18 Not
// exactly the same. IEEE-754 double cannot exactly represent
// 9223372036854775807, so the scientific notation represents the nearest
// floating-point value, not the exact integer.
