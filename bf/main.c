#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { INCP, DECP, INC, DEC, OUT, IN, JMP1, JMP2 } TOKEN_TYPE;

#define TOKEN_INCP '>'
#define TOKEN_DECP '<'
#define TOKEN_INC '+'
#define TOKEN_DEC '-'
#define TOKEN_OUT '.'
#define TOKEN_IN ','
#define TOKEN_JMP1 '['
#define TOKEN_JMP2 ']'

typedef struct Token {
    TOKEN_TYPE type;
    int info; // for repeating tokens (inc, dec), or jmp_pos for [, ]
} Token;

void token_fold(Token *prog, TOKEN_TYPE type, size_t *pos, size_t *prog_pos) {
    if (*pos == 0) {
        prog[*prog_pos] = (Token){.type = type, .info = 1};
        (*prog_pos)++;
    } else {
        if (prog[*prog_pos - 1].type == type) {
            prog[*prog_pos - 1].info += 1;
        } else {
            prog[*prog_pos] = (Token){.type = type, .info = 1};
            (*prog_pos)++;
        }
    }
    (*pos)++;
}

size_t tokenize(uint8_t *buf, Token *prog, size_t size) {
    size_t pos = 0;
    size_t prog_pos = 0;
    int prev_pos = prog_pos;
    while (pos < size) {
        switch (buf[pos]) {

        case TOKEN_INCP:
            token_fold(prog, INCP, &pos, &prog_pos);
            break;
        case TOKEN_DECP:
            token_fold(prog, DECP, &pos, &prog_pos);
            break;
        case TOKEN_INC:
            token_fold(prog, INC, &pos, &prog_pos);
            break;
        case TOKEN_DEC:
            token_fold(prog, DEC, &pos, &prog_pos);
            break;
        case TOKEN_OUT:
            token_fold(prog, OUT, &pos, &prog_pos);
            break;
        case TOKEN_IN:
            token_fold(prog, IN, &pos, &prog_pos);
            break;
        case TOKEN_JMP1:
            prog[prog_pos] = (Token){.type = JMP1, .info = -1};
            prog_pos++;
            pos++;
            break;
        case TOKEN_JMP2:
            for (prev_pos = prog_pos; prev_pos >= 0; prev_pos--) {
                if (prog[prev_pos].type == JMP1 && prog[prev_pos].info == -1)
                    break;
            }

            if (prev_pos == -1) {
                fprintf(stderr, "ERROR: Could not tokenize file. No '[' found for ']' at position: %zu\n", pos);
                exit(1);
            }

            prog[prev_pos].info = prog_pos;
            prog[prog_pos] = (Token){.type = JMP2, .info = prev_pos};
            prog_pos++;
            pos++;
            break;
        default:
            pos++;
        }
    }
    for (size_t i = 0; i < prog_pos; i++) {
        if (prog[i].type == JMP1 && prog[i].info == -1) {
            // TODO: incorrect position
            fprintf(stderr, "ERROR: Could not tokenize file. No ']' found for '[' at position: %zu\n", i);
            exit(1);
        }
    }
    return prog_pos;
}

void run(Token *prog, size_t size, size_t prog_size) {
    unsigned char *cells = malloc(sizeof(unsigned char) * size); // 1byte cells, max can be needed is size
    memset(cells, 0, sizeof(unsigned char) * size);

    size_t mem_pos = 0;
    size_t prog_pos = 0;
    while (prog_pos < prog_size) {
        Token token = prog[prog_pos];
        switch (token.type) {

        case INCP:
            mem_pos += token.info;
            if (mem_pos >= size)
                mem_pos -= size;
            prog_pos++;
            break;
        case DECP:
            mem_pos -= token.info;
            if (mem_pos < 0)
                mem_pos += size;
            prog_pos++;
            break;
        case INC:
            cells[mem_pos] += token.info;
            prog_pos++;
            break;
        case DEC:
            cells[mem_pos] -= token.info;
            prog_pos++;
            break;
        case OUT:
            for (int i = 0; i < token.info; i++)
                printf("%c", cells[mem_pos]);
            prog_pos++;
            break;
        case IN:
            for (int i = 0; i < token.info; i++)
                scanf("%s", &cells[mem_pos]);
            prog_pos++;
            break;
        case JMP1:
            if (cells[mem_pos] == 0) {
                prog_pos = token.info;
            } else {
                prog_pos++;
            }
            break;
        case JMP2:
            if (cells[mem_pos] != 0) {
                prog_pos = token.info;
            } else {
                prog_pos++;
            }
            break;
        }
    }
    // for (int i = 0; i < size; i++) {
    //     printf("%d", cells[i]);
    // }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./main <input_file_name>\n");
        exit(1);
    }

    FILE *input_file = fopen(argv[1], "r");
    if (input_file == NULL) {
        fprintf(stderr, "ERROR: Could not open file: %s\n", argv[1]);
        exit(1);
    }

    fseek(input_file, 0L, SEEK_END);
    long int size = ftell(input_file);
    fseek(input_file, 0L, SEEK_SET);

    uint8_t buf[size];

    size_t ret = fread(buf, sizeof(buf[0]), size, input_file);

    if (ret != size) {
        fprintf(stderr, "ERROR: Could not read file: %s\n", argv[1]);
        fclose(input_file);
        exit(1);
    }
    fclose(input_file);

    Token *prog = (Token *)malloc(sizeof(Token) * size); // max it needs without token folding

    size_t prog_size = tokenize(buf, prog, size);

    // for (int i = 0; i < prog_size; i++) {
    //     printf("Token type %d info %d, ", prog[i].type, prog[i].info);
    // }
    // printf("\n");
    run(prog, size, prog_size);

    return 0;
}
