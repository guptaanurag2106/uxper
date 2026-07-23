#include <stdio.h>

#define JSON_IMPLEMENTATION
#define ARENA_IMPLEMENTATION
#include "json.h"
#define UTILS_IMPLEMENTATION
#include "utils.h"

int main(void) {
    Json *root = json_parse_file("./tests/test_embedded.json");
    printf("json_parse_file: %s\n", root ? "OK" : "FAIL");
    if (!root) {
        printf("json_get_error: %s\n", json_get_error());
        return 1;
    }

    const Json *scene_crc = json_find(root, "scene_crc");
    printf("json_number(scene_crc): %f\n", json_number(scene_crc));

    const Json *scene_json = json_find(root, "scene_json");
    printf("json_string(scene_json): %s\n", json_cstring(scene_json));

    const Json *scene = json_parse_string(json_cstring(scene_json));
    printf("json_parse_string: %s\n", scene ? "OK" : "FAIL");
    if (!scene) {
        printf("json_get_error: %s\n", json_get_error());
        return 1;
    }

    json_free((Json *)scene_crc);
    json_free((Json *)root);
}
