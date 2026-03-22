#include <assert.h>
#include <stdio.h>
#include <string.h>

#define UTILS_IMPLEMENTATION
#include "../utils/utils.h"

int main(int argc, char **argv) {
    shift(&argc, &argv);
    assert(argc > 0 && "Expected file name");
    const char *file_name = shift(&argc, &argv);
    /* printf("Generating LLVM IR for %s\n", file_name); */

    char *file = read_entire_file(file_name);
    size_t len = strlen(file);

    int index = 1;

    int while_index = 0;
    int while_indices[1000];
    int *while_index_ptr = while_indices;

    // header
    printf("target triple = \"x86_64-pc-linux-gnu\"\n");
    printf("define i32 @main() {\n");
    printf("    %%ptr = alloca i8*, align 8\n");
    printf("    %%data_ptr = call i8* @calloc(i64 %zu, i64 1)\n",
           len);  // max len memory points
    printf("    store i8* %%data_ptr, i8** %%ptr, align 8\n");

    while (*file) {
        switch (*file) {
            case '>':
                printf("    %%%d = load i8*, i8** %%ptr, align 8\n", index);
                printf("    %%%d = getelementptr inbounds i8, i8* %%%d, i32 1\n",
                       index + 1, index);
                printf("    store i8* %%%d, i8** %%ptr, align 8\n", index + 1);
                index += 2;
                break;
            case '<':
                printf("    %%%d = load i8*, i8** %%ptr, align 8\n", index);
                printf("    %%%d = getelementptr inbounds i8, i8* %%%d, i32 -1\n",
                       index + 1, index);
                printf("    store i8* %%%d, i8** %%ptr, align 8\n", index + 1);
                index += 2;
                break;
            case '+':
                printf("    %%%d = load i8*, i8** %%ptr, align 8\n", index);
                printf("    %%%d = load i8, i8* %%%d, align 1\n", index + 1,
                       index);
                printf("    %%%d = add i8 %%%d, 1\n", index + 2, index + 1);
                printf("    store i8 %%%d, i8* %%%d, align 1\n", index + 2,
                       index);
                index += 3;
                break;
            case '-':
                printf("    %%%d = load i8*, i8** %%ptr, align 8\n", index);
                printf("    %%%d = load i8, i8* %%%d, align 1\n", index + 1,
                       index);
                printf("    %%%d = add i8 %%%d, -1\n", index + 2, index + 1);
                printf("    store i8 %%%d, i8* %%%d, align 1\n", index + 2,
                       index);
                index += 3;
                break;
            case '[':
                printf("    br label %%while_cond%d\n", while_index);
                printf("while_cond%d: \n", while_index);
                printf("    %%%d = load i8*, i8** %%ptr, align 8\n", index);
                printf("    %%%d = load i8, i8* %%%d, align 1\n", index + 1,
                       index);
                printf("    %%%d = icmp ne i8 %%%d, 0\n", index + 2, index + 1);
                printf(
                    "    br i1 %%%d, label %%while_body%d, label %%while_end%d\n",
                    index + 2, while_index, while_index);
                printf("while_body%d:\n", while_index);
                *while_index_ptr++ = while_index++;
                index += 3;
                break;
            case ']':
                if (--while_index_ptr < while_indices) {
                    fprintf(stderr, "unmatched ]\n");
                    return 1;
                }

                printf("    br label %%while_cond%d\n", *while_index_ptr);
                printf("while_end%d:\n", *while_index_ptr);

                break;
            case '.':
                printf("    %%%d = load i8*, i8** %%ptr, align 8\n", index);
                printf("    %%%d = load i8, i8* %%%d, align 1\n", index + 1,
                       index);
                printf("    %%%d = sext i8 %%%d to i32\n", index + 2, index + 1);
                printf("    %%%d = call i32 @putchar(i32 %%%d)\n", index + 3,
                       index + 2);
                index += 4;
                break;
            case ',':
                printf("    %%%d = call i32 @getchar()\n", index);
                printf("    %%%d = trunc i32 %%%d to i8\n", index + 1, index);
                printf("    %%%d = load i8*, i8** %%ptr, align 8\n", index + 2);
                printf("    store i8 %%%d, i8* %%%d, align 1\n", index + 1,
                       index + 2);
                index += 3;
                break;
        }
        file++;
    }

    printf(" call void @free(i8* %%data_ptr)\n");
    printf(" ret i32 0\n");
    printf("}\n");
    printf("declare i8* @calloc(i64, i64)\n");
    printf("declare void @free(i8*)\n");
    printf("declare i32 @putchar(i32)\n");
    printf("declare i32 @getchar()\n");

    return 0;
}
