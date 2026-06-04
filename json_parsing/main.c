#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define JSON_IMPLEMENTATION
#define ARENA_IMPLEMENTATION
#include "json.h"
#define UTILS_IMPLEMENTATION
#include <cJSON.h>

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
    Json *json = json_parse_string(scene_file_content);
    if (json == NULL) {
        printf("%s\n", json_get_error());
        free(scene_file_content);
        return 1;
    } else {
        // json_dump(json, stdout, false);
        gettimeofday(&end, NULL);
        json_free(json);
        printf("json.h time taken: %fms\n", timersub_ms(&end, &start));
    }

    // gettimeofday(&start, NULL);
    // cJSON *cjson = cJSON_Parse(scene_file_content);
    // if (cjson == NULL) {
    //     free(scene_file_content);
    //     return 1;
    // } else {
    // //     printf("%s\n", cJSON_Print(cjson));
    //     gettimeofday(&end, NULL);
    //     free(scene_file_content);
    //     printf("cJSON time taken: %fms\n", timersub_ms(&end, &start));
    // }
    // cJSON_Delete(cjson);

    return 0;
}
