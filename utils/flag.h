#ifndef FLAG_H_
#define FLAG_H_

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FLAG_DEF
#define FLAG_DEF inline static
#endif

#ifdef DEBUG
#define flag__ASSERT(x)                                                     \
    do {                                                                    \
        if (!(x)) {                                                         \
            fprintf(stderr, "Assertion failed: %s (%s:%d)\n", #x, __FILE__, \
                    __LINE__);                                              \
            abort();                                                        \
        }                                                                   \
    } while (0)
#else
#define flag__ASSERT(x) ((void)0)
#endif

#define MAX_FLAGS 256

struct flag__FlagOpts {
    bool required;
    char single_char;
};

FLAG_DEF const char *flag_program_name();

#define flag_int(name, default_value, usage, ...) \
    flag__int(name, default_value, usage, (struct flag__FlagOpts){__VA_ARGS__})

#define flag_int_var(result, name, default_value, usage, ...) \
    flag__int_var(result, name, default_value, usage,         \
                  (struct flag__FlagOpts){__VA_ARGS__})

#define flag_float(name, default_value, usage, ...) \
    flag__float(name, default_value, usage,         \
                (struct flag__FlagOpts){__VA_ARGS__})

#define flag_float_var(result, name, default_value, usage, ...) \
    flag__float_var(result, name, default_value, usage,         \
                    (struct flag__FlagOpts){__VA_ARGS__})

#define flag_bool(name, default_value, usage, ...) \
    flag__bool(name, default_value, usage, (struct flag__FlagOpts){__VA_ARGS__})

#define flag_bool_var(result, name, default_value, usage, ...) \
    flag__bool_var(result, name, default_value, usage,         \
                   (struct flag__FlagOpts){__VA_ARGS__})

#define flag_uint64(name, default_value, usage, ...) \
    flag__uint64(name, default_value, usage,         \
                 (struct flag__FlagOpts){__VA_ARGS__})

#define flag_uint64_var(result, name, default_value, usage, ...) \
    flag__uint64_var(result, name, default_value, usage,         \
                     (struct flag__FlagOpts){__VA_ARGS__})

#define flag_string(name, default_value, usage, ...) \
    flag__string(name, default_value, usage,         \
                 (struct flag__FlagOpts){__VA_ARGS__})

#define flag_string_var(result, name, default_value, usage, ...) \
    flag__string_var(result, name, default_value, usage,         \
                     (struct flag__FlagOpts){__VA_ARGS__})

FLAG_DEF void flag_parse(int argc, char **argv, const char *description);

FLAG_DEF void flag_usage();

// get remaining argc after parsing (after --);
FLAG_DEF int flag_get_rem_argc();

// get remaining argv after parsing (after --);
FLAG_DEF char **flag_get_rem_argv();

#ifdef __cplusplus
extern "C" {
#endif
#endif  // FLAG_H_

#ifdef FLAG_IMPLEMENTATION

static const char *flag__prog_name = NULL;
static int flag__argc;
static char **flag__argv;
static const char *flag__prog_description;

enum flag__FlagType {
    FLAG_INT = 0,
    FLAG_FLOAT,
    FLAG_STRING,
    FLAG_BOOL,
    FLAG_UINT64
};

typedef struct {
    const char *name;
    const char *usage;
    union {
        char **pval;
        int *ival;
        float *fval;
        bool *bval;
        uint64_t *u64val;
    } result;
    enum flag__FlagType type;
    bool required;
    bool found;
    char single_char;
} flag__Flag;

typedef struct {
    flag__Flag *flags;
    size_t size;
    size_t capacity;
} flag__Flags;

static flag__Flags flag__flags = {0};
static size_t flag__max_name_l = 0;
static size_t flag__required = 0;

FLAG_DEF char *flag__strdup(const char *s) {
    size_t len = strlen(s);
    char *d = (char *)malloc(len + 1);
    if (d == NULL) return NULL;
    memcpy(d, s, len + 1);
    return d;
}

FLAG_DEF void flag__vec_grow() {
    size_t new_cap =
        (flag__flags.capacity == 0) ? 16 : (flag__flags.capacity * 2);

    flag__flags.capacity = new_cap;

    flag__flags.flags =
        (flag__Flag *)realloc(flag__flags.flags, new_cap * sizeof(flag__Flag));
}

FLAG_DEF void flag__vec_push(flag__Flag flag) {
    if (flag__flags.size >= flag__flags.capacity) flag__vec_grow();
    flag__flags.flags[flag__flags.size++] = flag;
}

FLAG_DEF char *flag__shift(int *argc, char ***argv) {
    flag__ASSERT(*argc > 0);
    (*argc)--;
    return *((*argv)++);
}

FLAG_DEF void flag__print_try_help() {
    fprintf(stderr, "Try \"%s -h, --help\" for more options\n",
            flag__prog_name);
}

#define FLAG_DEFINE_VAR_FN(fn_name, flag_type, c_type, c_type_def, field,     \
                           assign)                                            \
    FLAG_DEF void fn_name(c_type result, const char *name,                    \
                          c_type_def default_value, const char *usage,        \
                          struct flag__FlagOpts opts) {                       \
        assign;                                                               \
        if (opts.single_char == 'h') {                                        \
            fprintf(                                                          \
                stderr,                                                       \
                "Cannot create a single character flag \"h\" for \"%s\", as " \
                "\"-h\" is already taken by \"--help\"\n",                    \
                name);                                                        \
            exit(1);                                                          \
        }                                                                     \
        size_t len = strlen(name) + (opts.single_char ? 3 : 0);               \
        if (len > flag__max_name_l) flag__max_name_l = len;                   \
        flag__required += opts.required;                                      \
        flag__vec_push((flag__Flag){.name = flag__strdup(name),               \
                                    .usage = flag__strdup(usage),             \
                                    .result.field = result,                   \
                                    .type = flag_type,                        \
                                    .required = opts.required,                \
                                    .found = false,                           \
                                    .single_char = opts.single_char});        \
    }
/* if (flag__flags.size > MAX_FLAGS)\ */

FLAG_DEFINE_VAR_FN(flag__int_var, FLAG_INT, int *, int, ival,
                   *result = default_value)

FLAG_DEFINE_VAR_FN(flag__float_var, FLAG_FLOAT, float *, float, fval,
                   *result = default_value)

FLAG_DEFINE_VAR_FN(flag__bool_var, FLAG_BOOL, bool *, bool, bval,
                   *result = default_value)

FLAG_DEFINE_VAR_FN(flag__uint64_var, FLAG_UINT64, uint64_t *, uint64_t, u64val,
                   *result = default_value)

FLAG_DEFINE_VAR_FN(flag__string_var, FLAG_STRING, char **, const char *, pval,
                   *result = flag__strdup(default_value))

#define FLAG_DEFINE_ALLOC_FN(fn_name, var_fn, ret_type, def_type, alloc_type)  \
    FLAG_DEF ret_type fn_name(const char *name, def_type default_value,        \
                              const char *usage, struct flag__FlagOpts opts) { \
        ret_type result = (ret_type)malloc(sizeof(alloc_type));                \
        var_fn(result, name, default_value, usage, opts);                      \
        return result;                                                         \
    }
FLAG_DEFINE_ALLOC_FN(flag__int, flag__int_var, int *, int, int)
FLAG_DEFINE_ALLOC_FN(flag__float, flag__float_var, float *, float, float)
FLAG_DEFINE_ALLOC_FN(flag__bool, flag__bool_var, bool *, bool, bool)
FLAG_DEFINE_ALLOC_FN(flag__uint64, flag__uint64_var, uint64_t *, uint64_t,
                     uint64_t)
FLAG_DEFINE_ALLOC_FN(flag__string, flag__string_var, char **, const char *,
                     char *)

FLAG_DEF void flag_parse(int argc, char **argv, const char *description) {
    flag__argc = argc;
    flag__argv = argv;
    flag__prog_description = description;
    flag__prog_name = flag__shift(&flag__argc, &flag__argv);

    while (flag__argc > 0) {
        const char *curr = flag__shift(&flag__argc, &flag__argv);
        /* printf("curr: %s\n", curr); */
        bool single_char_flag = true;
        if (*curr == '-') {
            if (*(curr + 1) == '-') {
                single_char_flag = false;
            }
        } else {
            fprintf(stderr, "Flag provided but not defined: %s\n", curr);
            exit(1);
        }

        curr += single_char_flag ? 1 : 2;
        if (!single_char_flag && *curr == '\0') {  // stop parsing flag after --
            return;
        }

        if (single_char_flag && strlen(curr) != 1) {
            if (strlen(curr) == 0) {
                fprintf(stderr, "Invalid flag: -\n");
            } else {
                fprintf(stderr, "Flag provided but not defined: -%s\n", curr);
            }
            flag__print_try_help();
            exit(1);
        }

        char *val = "";

        char *eqs = strstr(curr, "=");
        if (eqs == NULL) {
            if (flag__argc > 0) val = *(flag__argv);
        } else {
            val = eqs;
            *val = '\0';
            val++;
        }

        if (eqs == NULL) {
            if ((single_char_flag && *curr == 'h') ||
                (!single_char_flag && strcmp(curr, "help") == 0)) {
                flag_usage();
                exit(0);
            }
        }

        bool found_match = false;
        for (size_t i = 0; i < flag__flags.size; i++) {
            flag__Flag *flag = &flag__flags.flags[i];
            if (single_char_flag) {
                if (flag->single_char != *curr) continue;
            } else if (strcmp(flag->name, curr) != 0)
                continue;

            found_match = true;
            flag->found = true;

            switch (flag->type) {
                case FLAG_INT:
                    if (*val == '\0') {
                        fprintf(stderr, "Flag needs an argument: --%s\n", curr);
                        flag__print_try_help();
                        exit(1);
                    }
                    if (eqs == NULL) {
                        flag__shift(&flag__argc, &flag__argv);
                    }

                    /* printf("name: %s val: |%s|\n", curr, val); */
                    char *end_ptr;
                    long value;
                    errno = 0;
                    value = strtol(val, &end_ptr, 10);
                    if (*end_ptr != '\0' || errno == ERANGE) {
                        fprintf(
                            stderr,
                            "Invalid value \"%s\" for flag --%s: parse error\n",
                            val, curr);
                        flag__print_try_help();
                        exit(1);
                    }
                    if (value > INT_MAX || value < INT_MIN) {
                        fprintf(stderr,
                                "Out of range value \"%s\" for flag --%s: "
                                "parse error\n",
                                val, curr);
                        flag__print_try_help();
                        exit(1);
                    }
                    *flag->result.ival = (int)value;
                    break;
                case FLAG_UINT64:
                    if (*val == '\0') {
                        fprintf(stderr, "Flag needs an argument: --%s\n", curr);
                        flag__print_try_help();
                        exit(1);
                    }
                    if (eqs == NULL) {
                        flag__shift(&flag__argc, &flag__argv);
                    }

                    unsigned long long u64_value;
                    errno = 0;
                    u64_value = strtoull(val, &end_ptr, 0);
                    if (*end_ptr != '\0' || errno == ERANGE) {
                        fprintf(
                            stderr,
                            "Invalid value \"%s\" for flag --%s: parse error\n",
                            val, curr);
                        flag__print_try_help();
                        exit(1);
                    }
                    *flag->result.u64val = (uint64_t)u64_value;
                    break;
                case FLAG_FLOAT:
                    if (*val == '\0') {
                        fprintf(stderr, "Flag needs an argument: --%s\n", curr);
                        flag__print_try_help();
                        exit(1);
                    }
                    if (eqs == NULL) {
                        flag__shift(&flag__argc, &flag__argv);
                    }

                    /* printf("name: %s val: |%s|\n", curr, val); */
                    errno = 0;
                    *flag->result.fval = strtof(val, &end_ptr);
                    if (*end_ptr != '\0' || errno == ERANGE) {
                        fprintf(
                            stderr,
                            "Invalid value \"%s\" for flag --%s: parse error\n",
                            val, curr);
                        flag__print_try_help();
                        exit(1);
                    }
                    break;
                case FLAG_STRING:
                    if (eqs == NULL) {
                        if (flag__argc == 0) {
                            fprintf(stderr, "Flag needs an argument: --%s\n",
                                    curr);
                            flag__print_try_help();
                            exit(1);
                        }
                        flag__shift(&flag__argc, &flag__argv);
                    }

                    /* printf("name: %s val: |%s|\n", curr, val); */
                    *flag->result.pval = val;
                    break;
                case FLAG_BOOL:
                    *flag->result.bval = true;
                    if (eqs != NULL) {  // --flag=true | --flag=false
                        if (strcmp(val, "true") == 0) {
                        } else if (strcmp(val, "false") == 0) {
                            *flag->result.bval = false;
                        } else {
                            fprintf(
                                stderr,
                                "Invalid boolean value \"%s\" for --%s: parse "
                                "error\n",
                                val, curr);
                            flag__print_try_help();
                            exit(1);
                        }
                    } else if (strcmp(val, "true") == 0) {  // flag true
                        *flag->result.bval = true;
                        flag__shift(&flag__argc, &flag__argv);
                    } else if (strcmp(val, "false") == 0) {  // flag false
                        *flag->result.bval = false;
                        flag__shift(&flag__argc, &flag__argv);
                    }
                    /* printf("name: %s val: |%s|\n", curr, */
                    /*        *flag->result.bval ? "true" : "false"); */
                    break;
                default:
                    flag__ASSERT(0 && "Unreachable");
            }

            break;
        }
        if (!found_match) {
            if (single_char_flag)
                fprintf(stderr, "Flag provided but not defined: %s\n",
                        curr - 1);
            else
                fprintf(stderr, "Flag provided but not defined: %s\n",
                        curr - 2);

            flag__print_try_help();
            exit(1);
        }
    }

    for (size_t i = 0; i < flag__flags.size; i++) {
        flag__Flag flag = flag__flags.flags[i];
        if (flag.required && !flag.found) {
            fprintf(stderr, "Missing required flag \"--%s\": \"%s\"\n",
                    flag.name, flag.usage);
            flag__print_try_help();
            exit(1);
        }
    }
}

FLAG_DEF void flag__print_flag(flag__Flag *flag) {
    /* printf("%d", flag__max_name_l); */
    if (flag->single_char) {
        printf("  -%c, --%-*s", flag->single_char, (int)(flag__max_name_l - 4),
               flag->name);
    } else {
        printf("  --%-*s", (int)flag__max_name_l, flag->name);
    }

    switch (flag->type) {
        case FLAG_INT:
            printf(" int\n");
            break;
        case FLAG_FLOAT:
            printf(" float\n");
            break;
        case FLAG_STRING:
            printf(" string\n");
            break;
        case FLAG_BOOL:
            printf("\n");
            break;
        default:
            flag__ASSERT(0 && "Unreachable");
    }

    if (flag->usage != NULL && strlen(flag->usage) != 0)
        printf("      %s", flag->usage);

    switch (flag->type) {
        case FLAG_INT:
            printf(" (default %d)\n", *flag->result.ival);
            break;
        case FLAG_UINT64:
            printf(" (default %" PRIu64 ")\n", *flag->result.u64val);
            break;
        case FLAG_FLOAT:
            printf(" (default %f)\n", *flag->result.fval);
            break;
        case FLAG_STRING:
            if (strlen(*flag->result.pval) != 0)
                printf(" (default %s)\n", *flag->result.pval);
            else
                printf("\n");
            break;
        case FLAG_BOOL:
            printf("\n");
            break;
        default:
            flag__ASSERT(0 && "Unreachable");
    }
}

FLAG_DEF void flag_usage() {
    printf("Usage: %s\n", flag__prog_name);

    if (flag__prog_description != NULL && strlen(flag__prog_description) != 0) {
        printf("%s\n", flag__prog_description);
    }

    printf("\nOptions: \n");

    if (flag__required > 0) {
        printf("Required Arguments:\n");
        for (size_t i = 0; i < flag__flags.size; i++) {
            flag__Flag flag = flag__flags.flags[i];
            if (flag.required) flag__print_flag(&flag);
        }
    }
    if (flag__required < flag__flags.size) {
        printf("Optional Arguments:\n");
        for (size_t i = 0; i < flag__flags.size; i++) {
            flag__Flag flag = flag__flags.flags[i];
            if (!flag.required) flag__print_flag(&flag);
        }
    }
}

FLAG_DEF const char *flag_program_name() { return flag__prog_name; }

FLAG_DEF int flag_get_rem_argc() { return flag__argc; }
FLAG_DEF char **flag_get_rem_argv() { return flag__argv; }

#endif
