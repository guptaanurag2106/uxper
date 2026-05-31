#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UTILS_IMPLEMENTATION
#include "../utils/utils.h"
#define ARENA_IMPLEMENTATION
#include "../utils/arena.h"

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

typedef struct Json_DataS Json_DataS;
Vector(Json_DataS, Json_Arr_Data);
union Json_Data {
    Json_Arr_Data arrval;
    struct Json *jsonval;
    long numval;
    double realval;
    String stringval;
    bool boolval;
};
struct Json_DataS {
    enum Node_Kind data_kind;
    union Json_Data data;
};

typedef struct Json {
    char *input_file;
    struct Json *next;
    Arena *arena;  // shared for lexer, just 1 arena for lexer json tree etc

    const char *key;

    Json_DataS json_data;

    enum Node_Kind kind;
    bool is_toplevel;
} Json;

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

typedef struct {
    String data;
    const char *path;

    Arena *arena;
    Json *json;

    const char *curr;
    int line_number;
    int line_offset;

    enum Token_Kind kind;
    Json_DataS json_data;
} Lexer;

char *string_to_charp(Arena *arena, String data) {
    return arena_sprintf(arena, "%.*s", (int)data.len, data.start);
}

char *lexer_print_position(Lexer *l) {
    return arena_sprintf(l->arena, "%s:%d:%d", l->path, l->line_number,
                         l->line_offset);
}

char lexer_peek_char(Lexer *l) {  // peeks next char
    if (l->curr >= (l->data.start + l->data.len - 1)) {
        return '\x03';  // TODO:end of text ???
    }
    return *(l->curr + 1);
}

char lexer_get_char(Lexer *l) {  // returns current char and move next
    if (l->curr >= (l->data.start + l->data.len)) {
        return '\x03';  // TODO:end of text ???
    }
    char curr = *l->curr;
    if (curr == '\r') {
        if (lexer_peek_char(l) == '\n') {
            l->curr++;
        }
        curr = '\n';
        l->line_number++;
        l->line_offset = 0;
    } else if (curr == '\n') {
        l->line_number++;
        l->line_offset = 0;
    } else {
        l->line_offset++;
    }
    l->curr++;

    return curr;
}

// just compare if next char match input
bool lexer_expect_bytes(Lexer *l, const char *input) {
    int len = 0;
    while (true) {
        if (input[len] == '\0') return true;
        if ((l->curr + len) >= (l->data.start + l->data.len) ||
            *(l->curr + len) != input[len]) {
            return false;
        }
        len++;
    }
}

// compare if next char match input, if so increment curr
bool lexer_get_expect_bytes(Lexer *l, const char *input) {
    if (lexer_expect_bytes(l, input)) {
        size_t len = strlen(input);
        for (size_t i = 0; i < len; i++) {
            lexer_get_char(l);
        }
        return true;
    } else {
        return false;
    }
}

static char *json_error;

void json_set_error() {}

char *json_get_error(void) { return json_error; }

void lexer_get_token(Lexer *l) {
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
        case '"':  // get string
            lexer_get_char(l);
            char *start = l->curr;
            size_t len = 0;
            l->kind = TOKEN_STRING;
            char c = lexer_get_char(l);
            while (true) {
                if (c == '\\') {  // TODO: proper escaping
                    c = lexer_get_char(l);
                    len++;
                } else if (c == '"')
                    break;
                c = lexer_get_char(l);
                len++;
            }
            l->json_data.data_kind = JSON_STRING;
            l->json_data.data.stringval = (String){.start = start, .len = len};
            break;
        case '\n':
        case '\r':
        case ' ':
        case '\t':
            lexer_get_char(l);
            lexer_get_token(l);
            break;
        default:
            if (is_digit(chr) || lexer_expect_bytes(l, "-") ||
                lexer_expect_bytes(l, "+")) {
                // TODO: number formats e/E -+
                bool is_real = false;
                bool is_negative = false;
                if (lexer_get_expect_bytes(l, "-")) {
                    is_negative = true;
                } else if (lexer_get_expect_bytes(l, "+")) {
                    // already advances
                    is_negative = false;
                }
                const char *start = l->curr;
                chr = lexer_peek_char(l);

                while (true) {
                    if (is_digit(chr)) {
                        lexer_get_char(l);
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

                if (is_real) {
                    l->kind = TOKEN_REAL;
                    errno = 0;
                    char *end;
                    double realval = strtod(start, &end);
                    if (errno == ERANGE) {
                        fprintf(stderr, "%s: double overflow detected\n",
                                lexer_print_position(l));
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
                        fprintf(stderr, "%s: long overflow detected\n",
                                lexer_print_position(l));
                        return;
                    }
                    l->json_data.data_kind = JSON_NUMBER;
                    l->json_data.data.numval = numval * (is_negative ? -1 : 1);
                }
            } else if (lexer_expect_bytes(l, "null")) {
                lexer_get_expect_bytes(l, "null");
                l->kind = TOKEN_NULL;
                l->json_data.data_kind = JSON_NULL;
            } else if (lexer_expect_bytes(l, "true")) {
                lexer_get_expect_bytes(l, "true");
                l->kind = TOKEN_BOOL;
                l->json_data.data_kind = JSON_BOOL;
                l->json_data.data.boolval = true;
            } else if (lexer_expect_bytes(l, "false")) {
                lexer_get_expect_bytes(l, "false");
                l->kind = TOKEN_BOOL;
                l->json_data.data_kind = JSON_BOOL;
                l->json_data.data.boolval = false;
            } else if (l->curr == (l->data.start + l->data.len)) {
                l->kind = TOKEN_END;
            } else {
                l->kind = TOKEN_UNKNOWN;
                l->json_data.data.stringval =
                    (String){.start = l->curr, .len = 1};
            }
    }
}

char *lexer_print_token(Lexer *l) {
    switch (l->kind) {
        case TOKEN_OPENCURLY:
            return "{";
            break;
        case TOKEN_CLOSECURLY:
            return "}";
            break;
        case TOKEN_OPENSQUARE:
            return "[";
            break;
        case TOKEN_CLOSESQUARE:
            return "]";
            break;
        case TOKEN_COMMA:
            return ",";
            break;
        case TOKEN_COLON:
            return ":";
            break;
        case TOKEN_NUMBER:
            return arena_sprintf(l->arena, "%ld", l->json_data.data.numval);
        case TOKEN_REAL:
            return arena_sprintf(l->arena, "%f", l->json_data.data.realval);
        case TOKEN_STRING:
            return string_to_charp(l->arena, l->json_data.data.stringval);
            break;
        case TOKEN_BOOL:
            if (l->json_data.data.boolval) {
                return "true";
            }
            return "false";
            break;
        case TOKEN_NULL:
            return "null";
            break;
        case TOKEN_END:
            return "";
        case TOKEN_UNKNOWN:
            return string_to_charp(l->arena, l->json_data.data.stringval);
        default:
            ASSERT("unreachable token.kind");
            return "";
    }
}

static size_t __max(size_t a, size_t b) { return a > b ? a : b; }

Json *__parse_json(Lexer *l, bool toplevel);
bool __json_put_data(Json *json, Lexer *l);

bool __parse_json_array(Lexer *l, Json_Arr_Data *arr) {
    *arr = (Json_Arr_Data){0};
    while (true) {
        lexer_get_token(l);
        if (l->kind == TOKEN_CLOSESQUARE) {
            return true;
        }

        Json json = {0};
        if (!__json_put_data(&json, l)) {
            fprintf(stderr, "%s: expected json value got '%s'\n",
                    lexer_print_position(l), lexer_print_token(l));
            return false;
        }
        vec_push(arr, json.json_data);

        lexer_get_token(l);
        if (l->kind == TOKEN_CLOSESQUARE) {
            return true;
        }
        if (l->kind != TOKEN_COMMA) {
            fprintf(stderr, "%s: expected ',' or ']' got '%s'\n",
                    lexer_print_position(l), lexer_print_token(l));
            return false;
        }
    }
}

bool __json_put_data(Json *json, Lexer *l) {
    switch (l->kind) {
        case TOKEN_STRING:
        case TOKEN_NUMBER:
        case TOKEN_REAL:
        case TOKEN_BOOL:
        case TOKEN_NULL:
            json->kind = l->json_data.data_kind;
            json->json_data = l->json_data;
            break;
        case TOKEN_OPENCURLY:
            json->kind = JSON_OBJ;
            json->json_data.data_kind = JSON_OBJ;
            json->json_data.data.jsonval = __parse_json(l, false);
            break;
        case TOKEN_OPENSQUARE:
            json->kind = JSON_ARRAY;
            json->json_data.data_kind = JSON_ARRAY;
            if (!__parse_json_array(l, &json->json_data.data.arrval)) {
                return false;
            }
            break;
        default:
            return false;
    }
    return true;
}

Json *__parse_json(Lexer *l, bool toplevel) {
    Json *json = arena_alloc_struct_zeroed(l->arena, Json);
    if (json == NULL) {
        fprintf(stderr, "%s: error parsing from here, malloc failed\n",
                lexer_print_position(l));
    }

    if (toplevel) {
        if (!__json_put_data(json, l)) {
            fprintf(stderr, "%s: expected json value got '%s'\n",
                    lexer_print_position(l), lexer_print_token(l));
            return NULL;
        }
        return json;
    }

    lexer_get_token(l);
    switch (l->kind) {
        case TOKEN_STRING:
            json->key = string_to_charp(l->arena, l->json_data.data.stringval);
            lexer_get_token(l);
            if (l->kind != TOKEN_COLON) {
                fprintf(stderr, "%s: expected ':' got '%s'\n",
                        lexer_print_position(l), lexer_print_token(l));
                return NULL;
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
                    __json_put_data(json, l);
                    break;
                default:
                    fprintf(stderr, "%s: expected json value got '%s'\n",
                            lexer_print_position(l), lexer_print_token(l));
                    return NULL;
            }
            lexer_get_token(l);
            if (l->kind == TOKEN_CLOSECURLY) {
                return json;
            }
            if (l->kind != TOKEN_COMMA) {  // TODO: trailing comma
                fprintf(stderr, "%s: expected ',' got '%s'\n",
                        lexer_print_position(l), lexer_print_token(l));
                return NULL;
            }
            break;
        case TOKEN_CLOSECURLY:
            return NULL;
            break;
        case TOKEN_UNKNOWN:
            return NULL;
        default:
            fprintf(stderr, "%s: expected '}' got '%s'\n",
                    lexer_print_position(l), lexer_print_token(l));
            return NULL;
    }
    json->next = __parse_json(l, false);
    return json;
}

Json *parse_json(char *file_name) {
    Log(Log_Info, "parsing: %s", file_name);
    const char *data = read_entire_file(file_name);
    if (data == NULL) {
        fprintf(stderr, "Json: Could not read file %s\n", file_name);
        return NULL;
    }

    Arena *arena = malloc(sizeof(Arena));
    const size_t arena_size =
        __max(sizeof(Json) * 10, 20 * strlen(data));  // TODO: ??
    if (arena_create(arena, arena_size) < 0) {
        fprintf(stderr, "Json: Error could not malloc\n");
        return NULL;
    }

    Lexer lexer = {.arena = arena,
                   .data = (String){data, strlen(data)},
                   .curr = data,
                   .path = file_name,
                   .line_number = 1,
                   .line_offset = 0,
                   .json = NULL};

    lexer_get_token(&lexer);
    Json *json = __parse_json(&lexer, true);
    if (json == NULL) return NULL;
    lexer_get_token(&lexer);
    if (lexer.kind != TOKEN_END) {
        fprintf(stderr, "%s: unknown token expected EOF got '%s'\n",
                lexer_print_position(&lexer), lexer_print_token(&lexer));
        return NULL;
    }

    json->input_file = file_name;
    json->arena = arena;
    json->is_toplevel = true;
    return json;
}

void __dump_json(Json *json, FILE *f, int indent_len);
void __dump_json_data(Json_DataS *json_data, FILE *f, int indent_len) {
    switch (json_data->data_kind) {
        case JSON_NONE:
            return;
        case JSON_OBJ:
            fprintf(f, "{\n");
            indent_len += 4;
            __dump_json(json_data->data.jsonval, f, indent_len);
            indent_len -= 4;
            for (int i = 0; i < indent_len; i++) {
                fprintf(f, " ");
            }
            fprintf(f, "}");
            break;
        case JSON_ARRAY:
            fprintf(f, "[");
            size_t len = json_data->data.arrval.size;
            for (size_t i = 0; i < len; i++) {
                __dump_json_data(&json_data->data.arrval.items[i], f,
                                 indent_len);
                if (i != len - 1) {
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

void __dump_json(Json *json, FILE *f, int indent_len) {
    if (json == NULL) return;

    while (json != NULL) {
        for (int i = 0; i < indent_len; i++) {
            fprintf(f, " ");
        }

        if (!json->is_toplevel) {
            if (json->key == NULL) {
                Log(Log_Error, "dump_json: Null key encountered");
                return;
            }
            fprintf(f, "\"%s\": ", json->key);
        }

        __dump_json_data(&json->json_data, f, indent_len);
        if (json->next != NULL) fprintf(f, ",");
        fprintf(f, "\n");

        json = json->next;
    }
}

void dump_json(Json *json, FILE *f) { __dump_json(json, f, 0); }

void free_json(Json *json) {
    if (json != NULL && json->is_toplevel) {
        Log(Log_Info, "freeing: json object");
        arena_destroy(json->arena);
        free(json->arena);
    }
}

int main(int argc, char **argv) {
    const char *prog_name = shift(&argc, &argv);
    if (argc == 0) {
        printf("Usage: %s <input_toml_file>\n", prog_name);
        fprintf(stderr, "No input file provided\n");
        exit(1);
    }

    char *input_file = shift(&argc, &argv);
    Json *json = parse_json(input_file);
    if (json == NULL) return 1;

    Log(Log_Info, "dumping: %s", json->input_file);
    dump_json(json, stdout);
    free_json(json);

    return 0;
}
