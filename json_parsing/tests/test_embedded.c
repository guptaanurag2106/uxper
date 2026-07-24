#include <stdio.h>

#define JSON_IMPLEMENTATION
#define ARENA_IMPLEMENTATION
#include "json.h"
#define UTILS_IMPLEMENTATION
#include "utils.h"

static void print_json_error(const json_Error *err) {
    if (err == NULL) return;
    printf("%s:%d:%d: %s\n", err->source, err->line, err->column, err->message);
    return;
}

int main(void) {
    json_Error err = {0};
    Json *root = json_parse_file("./tests/test_embedded.json", &err);
    printf("json_parse_file: %s\n", root ? "OK" : "FAIL");
    if (!root) {
        print_json_error(&err);
        return 1;
    }

    const Json *scene_crc = json_find(root, "scene_crc");
    printf("json_number(scene_crc): %f\n", json_number(scene_crc));

    const Json *scene_json = json_find(root, "scene_json");
    printf("json_string(scene_json): %s\n", json_cstring(scene_json));

    json_Error scene_err = {0};
    const Json *scene = json_parse_string(json_cstring(scene_json), &scene_err);
    printf("json_parse_string: %s\n", scene ? "OK" : "FAIL");
    if (!scene) {
        print_json_error(&scene_err);
        return 1;
    }

    json_free((Json *)scene_crc);
    json_free((Json *)root);
}
