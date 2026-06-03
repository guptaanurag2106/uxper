#ifndef JSON_H
#define JSON_H

#include <stdbool.h>
#include <stddef.h>

#include "arena.h"

enum Node_Kind {
    JSON_NONE,
    JSON_OBJ,
    JSON_ARRAY,
    JSON_NUMBER,
    JSON_REAL,
    JSON_STRING,
    JSON_BOOL,
    JSON_NULL
};

typedef struct {
    const char *start;
    size_t len;
} String;

typedef struct Json_Value Json_Value;
// Vector(Json_Value, Json_Arr_Data);
typedef struct Json_Arr_Data {
    Json_Value *items;
    size_t size;
    size_t capacity;
} Json_Arr_Data;
union Json_Data {
    Json_Arr_Data arrval;
    struct Json *jsonval;
    long numval;
    double realval;
    String stringval;
    bool boolval;
};
struct Json_Value {
    union Json_Data data;
    enum Node_Kind data_kind;
};

typedef struct Json {
    Json_Value json_data;
    const char *key;
    const char *input_file;

    struct Json *next;
    Arena *arena;  // shared for lexer, just 1 arena for lexer json tree etc
    bool is_toplevel;
} Json;

typedef struct Lexer Lexer;

const char *json_get_error(void);

Json *json_parse_file(const char *file_name);

void json_dump(Json *json, FILE *f, bool minified);

void json_free(Json *json);

enum Node_Kind json_kind(const Json *node);

bool json_is(const Json *node, enum Node_Kind kind);

bool json_is_number(const Json *node);
bool json_is_real(const Json *node);
bool json_is_string(const Json *node);
bool json_is_bool(const Json *node);
bool json_is_null(const Json *node);
bool json_is_obj(const Json *node);
bool json_is_array(const Json *node);

const char *json_key(const Json *member);

const Json *json_next(const Json *member);

const Json *json_first(const Json *obj);

const Json *json_find(const Json *obj, const char *key);

long json_number(const Json *node);

double json_real(const Json *node);

const char *json_string(const Json *node);

bool json_bool(const Json *node);

size_t json_array_size(const Json *arr);

const Json_Value *json_array_at(const Json *arr, size_t i);

#ifdef JSON_IMPLEMENTATION

enum Token_Kind {
    TOKEN_OPENCURLY,
    TOKEN_CLOSECURLY,
    TOKEN_OPENSQUARE,
    TOKEN_CLOSESQUARE,
    TOKEN_COMMA,
    TOKEN_COLON,
    TOKEN_NUMBER,
    TOKEN_REAL,
    TOKEN_STRING,
    TOKEN_BOOL,
    TOKEN_NULL,
    TOKEN_END,
    TOKEN_UNKNOWN
};

struct Lexer {
    Json_Value json_data;
    Arena *arena;
    const char *path;

    const char *end;
    const char *curr;

    enum Token_Kind kind;

    int line_number;
    int line_offset;
};

#define JSON_ERROR_STR_SIZE 500
static char json_error[JSON_ERROR_STR_SIZE] = "";

#define json_set_error_raw(msg)               \
    do {                                      \
        memcpy(json_error, msg, strlen(msg)); \
        json_error[strlen(msg)] = '\0';       \
    } while (0)

#define json_set_error(format, args)                                     \
    do {                                                                 \
        int n = snprintf(json_error, JSON_ERROR_STR_SIZE, format, args); \
        if (n < 0 || n >= JSON_ERROR_STR_SIZE) {                         \
            fprintf(stderr, "Error message length exceeded %d bytes\n",  \
                    JSON_ERROR_STR_SIZE);                                \
            break;                                                       \
        }                                                                \
        json_error[n] = '\0';                                            \
    } while (0)

#define json_set_lerror_raw(l, format)                                         \
    do {                                                                       \
        int n = snprintf(json_error, JSON_ERROR_STR_SIZE, "%s:%d:%d: " format, \
                         l->path, l->line_number, l->line_offset);             \
        if (n < 0 || n >= JSON_ERROR_STR_SIZE) {                               \
            fprintf(stderr, "Error message length exceeded %d bytes\n",        \
                    JSON_ERROR_STR_SIZE);                                      \
        }                                                                      \
        json_error[n] = '\0';                                                  \
    } while (0)

#define json_set_lerror(l, format, args)                                       \
    do {                                                                       \
        int n = snprintf(json_error, JSON_ERROR_STR_SIZE, "%s:%d:%d: " format, \
                         l->path, l->line_number, l->line_offset, args);       \
        if (n < 0 || n >= JSON_ERROR_STR_SIZE) {                               \
            fprintf(stderr, "Error message length exceeded %d bytes\n",        \
                    JSON_ERROR_STR_SIZE);                                      \
        }                                                                      \
        json_error[n] = '\0';                                                  \
    } while (0)

const char *json_get_error(void) { return json_error; }

static char *arena_string_to_charp(Arena *arena, String data) {
    char *buf = arena_alloc_array(arena, char, data.len + 1);
    if (buf == NULL) return NULL;
    memcpy(buf, data.start, data.len);
    buf[data.len] = '\0';
    return buf;
}

static inline char lexer_peek_char(Lexer *l) {  // peeks next char
    if (l->curr >= (l->end)) {
        return '\x03';
    }
    return *(l->curr + 1);
}

static char lexer_get_char(Lexer *l) {  // returns current char and move next
    if (l->curr >= (l->end)) {
        return '\x03';
    }
    char curr = *l->curr;
    if (curr == '\r') {
        if (lexer_peek_char(l) == '\n') {
            l->curr++;
        }
        curr = '\n';
        l->line_number++;
        l->line_offset = 1;
    } else if (curr == '\n') {
        l->line_number++;
        l->line_offset = 1;
    } else {
        l->line_offset++;
    }
    l->curr++;

    return curr;
}

// just compare if next char match input
static bool lexer_expect_bytes(Lexer *l, const char *input, size_t strlen) {
    size_t len = 0;
    while (len < strlen) {
        if ((l->curr + len) >= (l->end) || *(l->curr + len) != input[len]) {
            return false;
        }
        len++;
    }
    return true;
}

// compare if next char match input, if so increment curr
static bool lexer_get_expect_bytes(Lexer *l, const char *input, size_t strlen) {
    if (lexer_expect_bytes(l, input, strlen)) {
        for (size_t i = 0; i < strlen; i++) {
            lexer_get_char(l);
        }
        return true;
    }
    return false;
}

static void lexer_get_token(Lexer *l) {
    char chr = *l->curr;
    switch (chr) {
        case '{':
            l->kind = TOKEN_OPENCURLY;
            lexer_get_char(l);
            break;
        case '}':
            l->kind = TOKEN_CLOSECURLY;
            lexer_get_char(l);
            break;
        case '[':
            l->kind = TOKEN_OPENSQUARE;
            lexer_get_char(l);
            break;
        case ']':
            l->kind = TOKEN_CLOSESQUARE;
            lexer_get_char(l);
            break;
        case ',':
            l->kind = TOKEN_COMMA;
            lexer_get_char(l);
            break;
        case ':':
            l->kind = TOKEN_COLON;
            lexer_get_char(l);
            break;
        case '"': {  // get string
            lexer_get_char(l);
            const char *start = l->curr;
            size_t len = 0;
            l->kind = TOKEN_STRING;
            char c = lexer_get_char(l);
            while (true) {
                if (c == '\\') {
                    c = lexer_get_char(l);
                    len++;
                } else if (c == '"')
                    break;
                c = lexer_get_char(l);
                len++;
            }
            l->json_data.data_kind = JSON_STRING;
            l->json_data.data.stringval = (String){.start = start, .len = len};
        } break;
        case '\n':
        case '\r':
        case ' ':
        case '\t':
            lexer_get_char(l);
            lexer_get_token(l);
            break;
        case '\x03':
            l->kind = TOKEN_END;
            break;
        default: {
            if ((chr >= '0' && chr <= '9') || lexer_expect_bytes(l, "-", 1) ||
                lexer_expect_bytes(l, "+", 1)) {
                // TODO: number formats e/E
                bool is_real = false;
                bool is_negative = false;
                bool eE_present = false;
                if (lexer_get_expect_bytes(l, "-", 1)) {
                    is_negative = true;
                } else if (lexer_get_expect_bytes(l, "+", 1)) {
                }
                const char *start = l->curr;
                chr = lexer_peek_char(l);

                while (true) {
                    if (chr >= '0' && chr <= '9') {
                        lexer_get_char(l);
                    } else if (chr == 'e' || chr == 'E') {
                        if (eE_present) break;
                        eE_present = true;
                        lexer_get_char(l);
                        if (lexer_peek_char(l) == '-' ||
                            lexer_peek_char(l) == '+') {
                            lexer_get_char(l);
                        }
                    } else if (chr == '.') {
                        if (is_real) {
                            break;
                        }
                        is_real = true;
                        lexer_get_char(l);
                    } else {
                        break;
                    }
                    chr = lexer_peek_char(l);
                }
                lexer_get_char(l);

                if (is_real || eE_present) {
                    l->kind = TOKEN_REAL;
                    errno = 0;
                    char *end;
                    double realval = strtod(start, &end);
                    if (errno == ERANGE) {
                        json_set_lerror_raw(l,
                                            "invalid number: double overflow");
                        l->kind = TOKEN_UNKNOWN;
                        return;
                    }
                    //"123." is valid strtod but not in json
                    if (*(end - 1) == '.') {
                        json_set_lerror_raw(
                            l, "invalid number: decimal cannot end with '.'");
                        l->kind = TOKEN_UNKNOWN;
                        return;
                    }
                    l->json_data.data_kind = JSON_REAL;
                    l->json_data.data.realval =
                        realval * (is_negative ? -1 : 1);
                } else {
                    l->kind = TOKEN_NUMBER;
                    errno = 0;
                    char *end;
                    long numval = strtol(start, &end, 10);
                    if (errno == ERANGE) {
                        l->kind = TOKEN_UNKNOWN;
                        json_set_lerror_raw(l, "invalid integer: overflow");
                        return;
                    }
                    if (*start == '0' && numval != 0) {
                        l->kind = TOKEN_UNKNOWN;
                        json_set_lerror_raw(l, "invalid integer: leading zero");
                        return;
                    }
                    l->json_data.data_kind = JSON_NUMBER;
                    l->json_data.data.numval = numval * (is_negative ? -1 : 1);
                }
            } else if (lexer_expect_bytes(l, "null", 1)) {
                lexer_get_expect_bytes(l, "null", 4);
                l->kind = TOKEN_NULL;
                l->json_data.data_kind = JSON_NULL;
            } else if (lexer_expect_bytes(l, "true", 1)) {
                lexer_get_expect_bytes(l, "true", 4);
                l->kind = TOKEN_BOOL;
                l->json_data.data_kind = JSON_BOOL;
                l->json_data.data.boolval = true;
            } else if (lexer_expect_bytes(l, "false", 1)) {
                lexer_get_expect_bytes(l, "false", 5);
                l->kind = TOKEN_BOOL;
                l->json_data.data_kind = JSON_BOOL;
                l->json_data.data.boolval = false;
            } else if (l->curr >= (l->end)) {
                l->kind = TOKEN_END;
            } else {
                l->kind = TOKEN_UNKNOWN;
                l->json_data.data.stringval =
                    (String){.start = l->curr, .len = 1};
            }
        }
    }
}

static char *lexer_print_token(Lexer *l) {
    switch (l->kind) {
        case TOKEN_OPENCURLY:
            return "{";
        case TOKEN_CLOSECURLY:
            return "}";
        case TOKEN_OPENSQUARE:
            return "[";
        case TOKEN_CLOSESQUARE:
            return "]";
        case TOKEN_COMMA:
            return ",";
        case TOKEN_COLON:
            return ":";
        case TOKEN_NUMBER:
            return arena_sprintf(l->arena, "%ld", l->json_data.data.numval);
        case TOKEN_REAL:
            return arena_sprintf(l->arena, "%f", l->json_data.data.realval);
        case TOKEN_STRING:
            return arena_string_to_charp(l->arena, l->json_data.data.stringval);
        case TOKEN_BOOL:
            if (l->json_data.data.boolval) {
                return "true";
            }
            return "false";
        case TOKEN_NULL:
            return "null";
        case TOKEN_END:
            return "";
        case TOKEN_UNKNOWN:
            return arena_string_to_charp(l->arena, l->json_data.data.stringval);
        default:
            assert(false && "unreachable token.kind");
            return "";
    }
}

static bool json_parse_object(Lexer *l, Json **res, bool toplevel);
static bool json_put_data(Json *json, Lexer *l);

static bool json_parse_array(Lexer *l, Json_Arr_Data *arr) {
    ArenaCheckpoint cp = arena_get_checkpoint(l->arena);
    *arr = (Json_Arr_Data){0};
    while (true) {
        lexer_get_token(l);
        if (l->kind == TOKEN_CLOSESQUARE) {
            return true;
        }

        Json json = {0};
        if (!json_put_data(&json, l)) {
            json_set_lerror(l, "expected json value, got '%s'",
                            lexer_print_token(l));
            arena_rewind(l->arena, cp);
            return false;
        }
        arena_vec_push(l->arena, arr, json.json_data);

        lexer_get_token(l);
        if (l->kind == TOKEN_CLOSESQUARE) {
            return true;
        }
        if (l->kind != TOKEN_COMMA) {
            json_set_lerror(l, "expected ',' or ']', got '%s'",
                            lexer_print_token(l));
            arena_rewind(l->arena, cp);
            return false;
        }
    }
}

static bool json_put_data(Json *json, Lexer *l) {
    switch (l->kind) {
        case TOKEN_STRING:
        case TOKEN_NUMBER:
        case TOKEN_REAL:
        case TOKEN_BOOL:
        case TOKEN_NULL:
            json->json_data = l->json_data;
            return true;
        case TOKEN_OPENCURLY:
            json->json_data.data_kind = JSON_OBJ;
            if (!json_parse_object(l, &json->json_data.data.jsonval, false)) {
                return false;
            }

            return true;
        case TOKEN_OPENSQUARE:
            json->json_data.data_kind = JSON_ARRAY;
            if (!json_parse_array(l, &json->json_data.data.arrval)) {
                return false;
            }
            return true;
        default:
            return false;
    }
}

static bool json_parse_object(Lexer *l, Json **res, bool toplevel) {
    ArenaCheckpoint cp = arena_get_checkpoint(l->arena);
    Json *json = arena_alloc_struct_zeroed(l->arena, Json);
    if (json == NULL) {
        json_set_lerror_raw(l, "out of memory");
    }
    json->arena = l->arena;
    if (toplevel) {
        if (!json_put_data(json, l)) {
            goto fail;
        }
        *res = json;
        return true;
    }

    lexer_get_token(l);
    switch (l->kind) {
        case TOKEN_STRING:
            json->key =
                arena_string_to_charp(l->arena, l->json_data.data.stringval);
            lexer_get_token(l);
            if (l->kind != TOKEN_COLON) {
                json_set_lerror(l, "expected ':', got '%s'",
                                lexer_print_token(l));
                goto fail;
            }
            lexer_get_token(l);
            switch (l->kind) {
                case TOKEN_STRING:
                case TOKEN_NUMBER:
                case TOKEN_REAL:
                case TOKEN_BOOL:
                case TOKEN_NULL:
                case TOKEN_OPENCURLY:
                case TOKEN_OPENSQUARE:
                    if (!json_put_data(json, l)) {
                        goto fail;
                    }
                    break;
                case TOKEN_UNKNOWN:
                    goto fail;
                default:
                    json_set_lerror(l, "expected json value, got '%s'",
                                    lexer_print_token(l));
                    goto fail;
            }
            lexer_get_token(l);
            if (l->kind != TOKEN_COMMA) {  // TODO: trailing comma
                json_set_lerror(l, "expected ',', got '%s'",
                                lexer_print_token(l));
                goto fail;
            }
            break;
        case TOKEN_CLOSECURLY:
            return true;
        default:
            json_set_lerror(l, "expected '}', got '%s'", lexer_print_token(l));
            goto fail;
    }
    if (!json_parse_object(l, &json->next, false)) {
        goto fail;
    }
    if (l->kind != TOKEN_CLOSECURLY) {
        json_set_lerror(l, "expected '}', got '%s'", lexer_print_token(l));
        goto fail;
    }
    *res = json;
    return true;

fail:
    *res = NULL;
    arena_rewind(l->arena, cp);
    return false;
}

static inline size_t max_size(size_t a, size_t b) { return a > b ? a : b; }

Json *json_parse_file(const char *file_name) {
    FILE *f = fopen(file_name, "r");
    if (f == NULL) {
        json_set_error("json: could not open file '%s'", file_name);
        return NULL;
    }

    if (fseek(f, 0, SEEK_END) < 0) {
        fclose(f);
        json_set_error("json: could not seek file '%s'", file_name);
        return NULL;
    }

    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    Arena *arena = malloc(sizeof(Arena));
    if (arena == NULL) {
        json_set_error_raw("json: out of memory");
        fclose(f);
        return NULL;
    }
    const size_t arena_size =
        max_size(sizeof(Json) * 64, (size_t)file_size * 15);
    if (arena_create(arena, arena_size) < 0) {
        json_set_error_raw("json: out of memory");
        fclose(f);
        return NULL;
    }

    char *data = arena_alloc_array(arena, char, (size_t)file_size + 1);
    if (data == NULL) {
        json_set_error_raw("json: out of memory");
        fclose(f);
        return NULL;
    }

    size_t bytes_read = fread(data, 1, (size_t)file_size, f);
    data[bytes_read] = '\0';
    fclose(f);

    Lexer lexer = {.arena = arena,
                   .end = data + bytes_read,
                   .curr = data,
                   .path = file_name,
                   .line_number = 1,
                   .line_offset = 1};

    lexer_get_token(&lexer);
    Json *json = NULL;
    if (!json_parse_object(&lexer, &json, true) || json == NULL) {
        return NULL;
    }
    lexer_get_token(&lexer);
    if (lexer.kind != TOKEN_END) {
        json_set_lerror((&lexer), "expected EOF, got '%s'",
                        lexer_print_token(&lexer));
        return NULL;
    }

    json->input_file = file_name;
    json->arena = arena;
    json->is_toplevel = true;
    return json;
}

static void json_dump_impl(Json *json, FILE *f, int indent_len, bool minified);
static void json_dump_data(Json_Value *json_data, FILE *f, int indent_len,
                           bool minified) {
    switch (json_data->data_kind) {
        case JSON_NONE:
            return;
        case JSON_OBJ:
            if (minified) {
                fprintf(f, "{");
            } else {
                fprintf(f, "{\n");
            }
            indent_len += 4;
            json_dump_impl(json_data->data.jsonval, f, indent_len, minified);
            indent_len -= 4;
            if (!minified) {
                for (int i = 0; i < indent_len; i++) {
                    fprintf(f, " ");
                }
            }
            fprintf(f, "}");
            break;
        case JSON_ARRAY:
            fprintf(f, "[");
            size_t len = json_data->data.arrval.size;
            for (size_t i = 0; i < len; i++) {
                json_dump_data(&json_data->data.arrval.items[i], f, indent_len,
                               minified);
                if (i != len - 1) {
                    if (minified)
                        fprintf(f, ",");
                    else
                        fprintf(f, ", ");
                }
            }
            fprintf(f, "]");
            break;
        case JSON_STRING:
            fprintf(f, "\"%.*s\"", (int)json_data->data.stringval.len,
                    json_data->data.stringval.start);
            break;
        case JSON_NUMBER:
            fprintf(f, "%ld", json_data->data.numval);
            break;
        case JSON_REAL:
            fprintf(f, "%lf", json_data->data.realval);
            break;
        case JSON_BOOL:
            fprintf(f, "%s", json_data->data.boolval ? "true" : "false");
            break;
        case JSON_NULL:
            fprintf(f, "null");
            break;
    }
}

static void json_dump_impl(Json *json, FILE *f, int indent_len, bool minified) {
    if (json == NULL) return;

    while (json != NULL) {
        if (!minified) {
            for (int i = 0; i < indent_len; i++) {
                fprintf(f, " ");
            }
        }

        if (!json->is_toplevel) {
            if (json->key == NULL) {
                json_set_error_raw("json_dump: null key encountered");
                return;
            }
            if (minified) {
                fprintf(f, "\"%s\":", json->key);
            } else {
                fprintf(f, "\"%s\": ", json->key);
            }
        }

        json_dump_data(&json->json_data, f, indent_len, minified);
        if (json->next != NULL) {
            if (minified)
                fprintf(f, ",");
            else
                fprintf(f, ", ");
        }
        if (!minified) {
            fprintf(f, "\n");
        }

        json = json->next;
    }
}

void json_dump(Json *json, FILE *f, bool minified) {
    json_dump_impl(json, f, 0, minified);
    if (minified) fprintf(f, "\n");
}

void json_free(Json *json) {
    if (json != NULL && json->is_toplevel) {
        Arena *arena = json->arena;
        arena_destroy(arena);
        free(arena);
    }
}

enum Node_Kind json_kind(const Json *node) {
    if (node == NULL) return JSON_NONE;
    return node->json_data.data_kind;
}

bool json_is(const Json *node, enum Node_Kind kind) {
    return json_kind(node) == kind;
}

bool json_is_number(const Json *node) { return json_is(node, JSON_NUMBER); }
bool json_is_real(const Json *node) { return json_is(node, JSON_REAL); }
bool json_is_string(const Json *node) { return json_is(node, JSON_STRING); }
bool json_is_bool(const Json *node) { return json_is(node, JSON_BOOL); }
bool json_is_null(const Json *node) { return json_is(node, JSON_NULL); }
bool json_is_obj(const Json *node) { return json_is(node, JSON_OBJ); }
bool json_is_array(const Json *node) { return json_is(node, JSON_ARRAY); }

const char *json_key(const Json *member) {
    if (member == NULL) return NULL;
    return member->key;
}

const Json *json_next(const Json *member) {
    if (member == NULL) return NULL;
    return member->next;
}

const Json *json_first(const Json *obj) {
    if (obj == NULL) return NULL;
    return obj->json_data.data.jsonval;
}

const Json *json_find(const Json *obj, const char *key) {
    if (obj == NULL || key == NULL) return NULL;
    for (const Json *it = json_first(obj); it != NULL; it = json_next(it)) {
        if (it->key != NULL && strcmp(it->key, key) == 0) return it;
    }
    return NULL;
}

long json_number(const Json *node) {
    if (node == NULL) return 0;
    return node->json_data.data.numval;
}

double json_real(const Json *node) {
    if (node == NULL) return 0.0;
    return node->json_data.data.realval;
}

const char *json_string(const Json *node) {
    if (node == NULL) return NULL;
    return arena_string_to_charp(node->arena, node->json_data.data.stringval);
}

bool json_bool(const Json *node) {
    if (node == NULL) return false;
    return node->json_data.data.boolval;
}

size_t json_array_size(const Json *arr) {
    if (arr == NULL) return 0;
    return arr->json_data.data.arrval.size;
}

const Json_Value *json_array_at(const Json *arr, size_t i) {
    if (arr == NULL) return NULL;
    if (i >= arr->json_data.data.arrval.size) return NULL;
    return &arr->json_data.data.arrval.items[i];
}
#endif

#endif  // JSON_H
