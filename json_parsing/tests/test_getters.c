#include <stdio.h>
#include <string.h>

#define JSON_IMPLEMENTATION
#define ARENA_IMPLEMENTATION
#include "json.h"

static void print_json_error(const json_Error *err) {
    if (err == NULL) return;
    printf("%s:%d:%d: %s\n", err->source, err->line, err->column, err->message);
    return;
}

static const char *kind_str(enum Node_Kind k) {
    switch (k) {
        case JSON_NONE:
            return "JSON_NONE";
        case JSON_OBJ:
            return "JSON_OBJ";
        case JSON_ARRAY:
            return "JSON_ARRAY";
        case JSON_INTEGER:
            return "JSON_INTEGER";
        case JSON_REAL:
            return "JSON_REAL";
        case JSON_STRING:
            return "JSON_STRING";
        case JSON_BOOL:
            return "JSON_BOOL";
        case JSON_NULL:
            return "JSON_NULL";
        default:
            return "UNKNOWN";
    }
}

#define SECTION(title) printf("\n=== %s ===\n", title)

int main(void) {
    SECTION("parse sample.json");
    json_Error err = {0};
    Json *root = json_parse_file("tests/sample.json", &err);
    printf("json_parse_file: %s\n", root ? "OK" : "FAIL");
    if (!root) {
        print_json_error(&err);
        return 1;
    }

    SECTION("json_kind on top-level and nested");
    printf("json_kind(root): %s\n", kind_str(json_kind(root)));

    const Json *meta = json_find(root, "meta");
    printf("json_kind(meta): %s\n", kind_str(json_kind(meta)));

    const Json *version = json_find(meta, "version");
    printf("json_kind(version): %s\n", kind_str(json_kind(version)));

    const Json *valid = json_find(meta, "valid");
    printf("json_kind(valid): %s\n", kind_str(json_kind(valid)));

    const Json *null_val = json_find(meta, "nullValue");
    printf("json_kind(nullValue): %s\n", kind_str(json_kind(null_val)));

    const Json *zero = json_find(json_find(root, "numbers"), "zero");
    printf("json_kind(zero): %s\n", kind_str(json_kind(zero)));

    const Json *flt = json_find(json_find(root, "numbers"), "float");
    printf("json_kind(float): %s\n", kind_str(json_kind(flt)));

    const Json *empty_arr = json_find(json_find(root, "arrays"), "empty");
    printf("json_kind(empty array): %s\n", kind_str(json_kind(empty_arr)));

    SECTION("json_is_* checks");
    printf("json_is_obj(root): %d\n", json_is_obj(root));
    printf("json_is_array(root): %d\n", json_is_array(root));
    printf("json_is_string(version): %d\n", json_is_string(version));
    printf("json_is_integer(zero): %d\n", json_is_integer(zero));
    printf("json_is_real(flt): %d\n", json_is_real(flt));
    printf("json_is_number(zero): %d\n", json_is_number(zero));
    printf("json_is_number(flt): %d\n", json_is_number(flt));
    printf("json_is_bool(valid): %d\n", json_is_bool(valid));
    printf("json_is_null(null_val): %d\n", json_is_null(null_val));
    printf("json_is_array(empty_arr): %d\n", json_is_array(empty_arr));

    SECTION("json_key");
    printf("json_key(meta): %s\n", json_key(meta));
    printf("json_key(version): %s\n", json_key(version));
    printf("json_key(root): %s\n", json_key(root));

    SECTION("json_find positive");
    const Json *numbers = json_find(root, "numbers");
    printf("json_find(root, \"numbers\"): %s\n",
           json_find(root, "numbers") ? "found" : "not found");

    const Json *positive = json_find(numbers, "positive");
    printf("json_find(numbers, \"positive\"): %s\n",
           positive ? "found" : "not found");

    const Json *booleans = json_find(root, "booleans");
    printf("json_find(root, \"booleans\"): %s\n",
           booleans ? "found" : "not found");

    const Json *arrays = json_find(root, "arrays");
    printf("json_find(root, \"arrays\"): %s\n", arrays ? "found" : "not found");

    const Json *final_obj = json_find(root, "final");
    printf("json_find(root, \"final\"): %s\n",
           final_obj ? "found" : "not found");

    SECTION("json_find negative / missing keys");
    printf("json_find(root, \"nonexistent\"): %s\n",
           json_find(root, "nonexistent") ? "found" : "not found");
    printf("json_find(root, \"\"): %s\n",
           json_find(root, "") ? "found" : "not found");
    printf("json_find(root, \"Meta\"): %s\n",
           json_find(root, "Meta") ? "found" : "not found");
    printf("json_find(NULL, \"meta\"): %s\n",
           json_find(NULL, "meta") ? "found" : "not found");

    SECTION("json_find on non-object");
    printf("json_find(version, \"x\"): %s\n",
           json_find(version, "x") ? "found" : "not found");

    // TODO: heh??
    SECTION("json_value_find");
    const Json_Value *root_val = &root->json_data;
    printf("json_value_find(&root->json_data, \"meta\"): %s\n",
           json_value_find(root_val, "meta") ? "found" : "not found");
    printf("json_value_find(NULL, \"meta\"): %s\n",
           json_value_find(NULL, "meta") ? "found" : "not found");
    printf("json_value_find(root_val, NULL): %s\n",
           json_value_find(root_val, NULL) ? "found" : "not found");

    SECTION("json_integer");
    printf("json_integer(zero): %ld\n", json_integer(zero));
    printf("json_integer(positive): %ld\n", json_integer(positive));
    const Json *negative = json_find(numbers, "negative");
    printf("json_integer(negative): %ld\n", json_integer(negative));
    const Json *large = json_find(numbers, "largeInt");
    printf("json_integer(largeInt): %ld\n", json_integer(large));
    const Json *small = json_find(numbers, "smallInt");
    printf("json_integer(smallInt): %ld\n", json_integer(small));
    printf("json_integer(NULL): %ld\n", json_integer(NULL));

    SECTION("json_real");
    printf("json_real(flt): %g\n", json_real(flt));
    const Json *neg_flt = json_find(numbers, "negativeFloat");
    printf("json_real(negativeFloat): %g\n", json_real(neg_flt));
    const Json *sci_small = json_find(numbers, "scientificSmall");
    printf("json_real(scientificSmall): %g\n", json_real(sci_small));
    const Json *sci_neg = json_find(numbers, "scientificNegative");
    printf("json_real(scientificNegative): %g\n", json_real(sci_neg));
    printf("json_real(NULL): %g\n", json_real(NULL));

    SECTION("json_number");
    printf("json_number(zero): %g\n", json_number(zero));
    printf("json_number(positive): %g\n", json_number(positive));
    printf("json_number(flt): %g\n", json_number(flt));
    printf("json_number(neg_flt): %g\n", json_number(neg_flt));
    printf("json_number(sci_small): %g\n", json_number(sci_small));
    printf("json_number(NULL): %g\n", json_number(NULL));

    SECTION("json_cstring");
    printf("json_cstring(version): %s\n", json_cstring(version));
    const Json *escaped = json_find(meta, "escapedString");
    printf("json_cstring(escapedString): %s\n", json_cstring(escaped));
    const Json *empty_str = json_find(meta, "emptyString");
    printf("json_cstring(emptyString): \"%s\"\n", json_cstring(empty_str));
    const Json *unicode = json_find(meta, "unicode");
    printf("json_cstring(unicode): %s\n", json_cstring(unicode));
    printf("json_cstring(NULL): %s\n",
           json_cstring(NULL) ? "non-null" : "NULL");
    printf("json_cstring(zero) [integer]: %s\n",
           json_cstring(zero) ? "non-null" : "NULL");

    SECTION("json_bool");
    printf("json_bool(valid): %d\n", json_bool(valid));
    const Json *yes = json_find(booleans, "yes");
    const Json *no = json_find(booleans, "no");
    printf("json_bool(yes): %d\n", json_bool(yes));
    printf("json_bool(no): %d\n", json_bool(no));
    printf("json_bool(NULL): %d\n", json_bool(NULL));

    SECTION("json_array_size");
    const Json *nums_arr = json_find(arrays, "numbers");
    const Json *strs_arr = json_find(arrays, "strings");
    const Json *mixed_arr = json_find(arrays, "mixed");
    const Json *obj_arr = json_find(root, "objectArray");
    const Json *matrix = json_find(root, "matrix");
    const Json *events = json_find(root, "events");
    const Json *records = json_find(root, "records");
    printf("json_array_size(empty): %zu\n", json_array_size(empty_arr));
    printf("json_array_size(numbers): %zu\n", json_array_size(nums_arr));
    printf("json_array_size(strings): %zu\n", json_array_size(strs_arr));
    printf("json_array_size(mixed): %zu\n", json_array_size(mixed_arr));
    printf("json_array_size(objectArray): %zu\n", json_array_size(obj_arr));
    printf("json_array_size(matrix): %zu\n", json_array_size(matrix));
    printf("json_array_size(records): %zu\n", json_array_size(records));
    printf("json_array_size(NULL): %zu\n", json_array_size(NULL));
    printf("json_array_size(root) [object]: %zu\n", json_array_size(root));

    SECTION("json_array_at");
    const Json_Value *at0 = json_array_at(nums_arr, 0);
    const Json_Value *at1 = json_array_at(nums_arr, 1);
    const Json_Value *at2 = json_array_at(nums_arr, 2);
    const Json_Value *at_end = json_array_at(nums_arr, 100);
    printf("json_array_at(numbers, 0) data_kind: %s\n",
           at0 ? kind_str(at0->data_kind) : "NULL");
    printf("json_array_at(numbers, 1) data_kind: %s\n",
           at1 ? kind_str(at1->data_kind) : "NULL");
    printf("json_array_at(numbers, 2) data_kind: %s\n",
           at2 ? kind_str(at2->data_kind) : "NULL");
    printf("json_array_at(numbers, 100): %s\n", at_end ? "found" : "NULL");
    printf("json_array_at(NULL, 0): %s\n",
           json_array_at(NULL, 0) ? "found" : "NULL");
    printf("json_array_at(root, 0) [object]: %s\n",
           json_array_at(root, 0) ? "found" : "NULL");

    SECTION("json_array_at values (numbers)");
    for (size_t i = 0; i < json_array_size(nums_arr); i++) {
        const Json_Value *v = json_array_at(nums_arr, i);
        printf("json_array_at(numbers, %zu) kind=%s\n", i,
               v ? kind_str(v->data_kind) : "NULL");
    }

    SECTION("json_first / json_next on object");
    const Json *it = json_first(root);
    int count = 0;
    while (it != NULL) {
        printf("member[%d] key=%s\n", count, json_key(it));
        count++;
        it = json_next(it);
    }
    printf("total members: %d\n", count);

    SECTION("json_first / json_next on meta");
    it = json_first(meta);
    count = 0;
    while (it != NULL) {
        printf("meta.member[%d] key=%s kind=%s\n", count, json_key(it),
               kind_str(json_kind(it)));
        count++;
        it = json_next(it);
    }

    SECTION("json_first edge cases");
    printf("json_first(NULL): %s\n", json_first(NULL) ? "non-null" : "NULL");
    printf("json_first(version) [string]: %s %s\n",
           json_first(version) ? "non-null" : "NULL",
           kind_str(json_kind(json_first(version))));
    printf("json_first(empty_arr) [array]: %s\n",
           json_first(empty_arr) ? "non-null" : "NULL");

    SECTION("json_next edge cases");
    printf("json_next(NULL): %s\n", json_next(NULL) ? "non-null" : "NULL");

    SECTION("deep nesting traversal");
    const Json *nested = json_find(root, "nested");
    const Json *l1 = json_find(nested, "level1");
    const Json *l2 = json_find(l1, "level2");
    const Json *l3 = json_find(l2, "level3");
    const Json *msg = json_find(l3, "message");
    printf("nested.level1.level2.level3.message: %s\n", json_cstring(msg));
    const Json *list = json_find(l3, "list");
    printf("nested.level1.level2.level3.list kind: %s\n",
           kind_str(json_kind(list)));
    printf("nested.level1.level2.level3.list size: %zu\n",
           json_array_size(list));

    SECTION("matrix traversal");
    printf("matrix size: %zu\n", json_array_size(matrix));
    for (size_t i = 0; i < json_array_size(matrix); i++) {
        const Json_Value *row = json_array_at(matrix, i);
        if (row && row->data_kind == JSON_ARRAY) {
            printf("matrix[%zu] size=%zu", i, row->data.arrval.size);
            for (size_t j = 0; j < row->data.arrval.size; j++) {
                const Json_Value *cell = &row->data.arrval.items[j];
                if (cell->data_kind == JSON_INTEGER) {
                    printf(" %ld", cell->data.intval);
                } else if (cell->data_kind == JSON_REAL) {
                    printf(" %g", cell->data.realval);
                }
            }
            printf("\n");
        }
    }

    SECTION("objectArray traversal");
    for (size_t i = 0; i < json_array_size(obj_arr); i++) {
        const Json_Value *v = json_array_at(obj_arr, i);
        if (v && v->data_kind == JSON_OBJ) {
            const Json *id = json_value_find(v, "id");
            const Json *name = json_value_find(v, "name");
            const Json *active = json_value_find(v, "active");
            const Json *score = json_value_find(v, "score");
            const Json *roles = json_value_find(v, "roles");
            const Json *not_found = json_value_find(v, "notFound");
            printf(
                "objectArray[%zu]: id=%ld name=%s active=%d score=%g "
                "roles_size = %zu not_found = %s\n",
                i, json_integer(id), json_cstring(name), json_bool(active),
                json_number(score), json_array_size(roles),
                json_cstring(not_found));
        }
    }

    SECTION("mixed array element kinds");
    for (size_t i = 0; i < json_array_size(mixed_arr); i++) {
        const Json_Value *v = json_array_at(mixed_arr, i);
        printf("mixed[%zu] kind=%s\n", i, v ? kind_str(v->data_kind) : "NULL");
    }

    SECTION("records traversal");
    for (size_t i = 0; i < json_array_size(records); i++) {
        const Json_Value *rec = json_array_at(records, i);
        if (rec && rec->data_kind == JSON_OBJ) {
            const Json *key = json_value_find(rec, "key");
            const Json *val = json_value_find(rec, "value");
            printf("records[%zu] key=%s value.kind=%s\n", i, json_cstring(key),
                   val ? kind_str(json_kind(val)) : "NULL");
            if (val) {
                const Json *en = json_find(val, "enabled");
                const Json *th = json_find(val, "threshold");
                printf("  value.enabled=%d value.threshold=%g\n", json_bool(en),
                       json_number(th));
            }
        }
    }

    SECTION("config deep query");
    const Json *cfg = json_find(root, "config");
    const Json *retry = json_find(cfg, "retry");
    const Json *timeout = json_find(cfg, "timeoutMs");
    const Json *features = json_find(cfg, "features");
    const Json *logging = json_find(features, "logging");
    const Json *metrics = json_find(features, "metrics");
    const Json *cache = json_find(features, "cache");
    const Json *cache_en = json_find(cache, "enabled");
    const Json *ttl = json_find(cache, "ttl");
    printf("config.retry: %ld\n", json_integer(retry));
    printf("config.timeoutMs: %ld\n", json_integer(timeout));
    printf("config.features.logging: %d\n", json_bool(logging));
    printf("config.features.metrics: %d\n", json_bool(metrics));
    printf("config.features.cache.enabled: %d\n", json_bool(cache_en));
    printf("config.features.cache.ttl: %ld\n", json_integer(ttl));

    SECTION("events traversal");
    for (size_t i = 0; i < json_array_size(events); i++) {
        const Json_Value *ev = json_array_at(events, i);
        if (ev && ev->data_kind == JSON_OBJ) {
            const Json *ts = json_value_find(ev, "timestamp");
            const Json *ty = json_value_find(ev, "type");
            const Json *pl = json_value_find(ev, "payload");
            printf("events[%zu] type=%s timestamp=%s payload.kind=%s\n", i,
                   json_cstring(ty), json_cstring(ts),
                   pl ? kind_str(json_kind(pl)) : "NULL");
        }
    }

    SECTION("edgeCases");
    const Json *edge = json_find(root, "edgeCases");
    const Json *empty_obj = json_find(edge, "emptyObject");
    const Json *single = json_find(edge, "singleItemArray");
    const Json *repeated = json_find(edge, "repeated");
    printf("edgeCases.emptyObject kind: %s\n", kind_str(json_kind(empty_obj)));
    printf("edgeCases.emptyObject first: %s\n",
           json_first(empty_obj) ? "non-null" : "NULL");
    printf("edgeCases.singleItemArray size: %zu\n", json_array_size(single));
    printf("edgeCases.repeated size: %zu\n", json_array_size(repeated));
    for (size_t i = 0; i < json_array_size(repeated); i++) {
        const Json_Value *v = json_array_at(repeated, i);
        if (v && v->data_kind == JSON_OBJ) {
            const Json *vv = json_value_find(v, "v");
            printf("edgeCases.repeated[%zu].v: %ld\n", i, json_integer(vv));
        }
    }

    SECTION("special strings");
    const Json *special = json_find(root, "special");
    printf("special.jsonLike: %s\n",
           json_cstring(json_find(special, "jsonLike")));
    printf("special.xmlLike: %s\n",
           json_cstring(json_find(special, "xmlLike")));
    String sqlLike = json_string_view(json_find(special, "sqlLike"));
    printf("special.sqlLike: %.*s\n", (int)sqlLike.len, sqlLike.start);

    printf("special.path: %s\n", json_cstring(json_find(special, "path")));
    printf("special.url: %s\n", json_cstring(json_find(special, "url")));
    printf("special.email: %s\n", json_cstring(json_find(special, "email")));

    SECTION("json_kind on NULL and bad inputs");
    printf("json_kind(NULL): %s\n", kind_str(json_kind(NULL)));

    json_free(root);
    return 0;
}
