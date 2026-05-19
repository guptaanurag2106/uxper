#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *arr =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

uint8_t find_index(char c) {
    for (uint8_t i = 0; i < 64; i++) {
        if (arr[i] == c) return i;
    }
    return 0xFF;
}

char *decode(const char *string) {
    size_t len = strlen(string);
    if (len % 4 != 0) {
        fprintf(stderr, "Error: Not a valid base64 encoded string\n");
        return "";
    }

    size_t out_len = len * 4 / 3;
    for (size_t i = len - 1; i >= 0; i--) {
        if (string[i] == '=')
            out_len--;
        else
            break;
    }
    char *ans = malloc(out_len + 1);
    if (ans == NULL) {
        fprintf(stderr, "Error: malloc failed, exiting.\n");
        exit(1);
    }
    ans[out_len] = '\0';

    size_t ans_index = 0;
    for (size_t i = 0; i < len; i += 4) {
        uint8_t b0 = find_index(string[i]), b1 = find_index(string[i + 1]),
                b2 = find_index(string[i + 2]), b3 = find_index(string[i + 3]);

        uint32_t final = (uint32_t)((b0 << 18) | (b1 << 12) | (b2 << 6) | b3);
        int num = (final >> 16) & 0xFF;
        ans[ans_index++] = (char)num;
        num = (final >> 8) & 0xFF;
        if (b2 != 0xFF) ans[ans_index++] = (char)num;
        num = (final) & 0xFF;
        if (b3 != 0xFF) ans[ans_index++] = (char)num;
    }

    return ans;
}

char *encode(const char *string) {
    size_t len = strlen(string);
    size_t out_len = 4 * (len + 2) / 3;
    char *ans = malloc(out_len + 1);
    if (ans == NULL) {
        fprintf(stderr, "Error: malloc failed, exiting.\n");
        exit(1);
    }
    ans[out_len] = '\0';

    size_t ans_index = 0;
    size_t equals_count = 3 - len % 3;
    equals_count = equals_count == 3 ? 0 : equals_count;

    for (size_t i = 0; i < len; i += 3) {
        uint8_t b0 = (uint8_t)string[i], b1 = 0, b2 = 0;
        if (i + 1 < len) {
            b1 = (uint8_t)string[i + 1];
        }
        if (i + 2 < len) {
            b2 = (uint8_t)string[i + 2];
        }

        uint32_t final =
            ((uint32_t)b0 << 16) | ((uint32_t)b1 << 8) | (uint32_t)b2;

        int num = (final >> 18) & 0x3F;
        ans[ans_index++] = arr[num];
        num = (final >> 12) & 0x3F;
        ans[ans_index++] = arr[num];
        num = (final >> 6) & 0x3F;
        ans[ans_index++] = arr[num];
        num = final & 0x3F;
        ans[ans_index++] = arr[num];
    }

    if (equals_count == 1) {
        ans[ans_index - 1] = '=';
    } else if (equals_count == 2) {
        ans[ans_index - 1] = '=';
        ans[ans_index - 2] = '=';
    }

    return ans;
}

char *encode2(const char *string) {
    size_t len = strlen(string);
    size_t out_len = 4 * (len + 2) / 3;
    char *ans = malloc(out_len + 1);
    if (ans == NULL) {
        fprintf(stderr, "Error: malloc failed, exiting.\n");
        exit(1);
    }
    ans[out_len] = '\0';

    int rem = 0, digits = 0;

    int ans_index = 0;
    for (size_t i = 0; i < len; i++) {
        int num;
        if (digits > 6) {
            num = rem;
            i--;
        } else {
            num = string[i] + rem * (1 << 8);
            digits += 8;
        }

        int num2 = 0;
        for (int j = 1; j <= 6; j++) {
            num2 += num & (1 << (digits - j));
        }
        digits -= 6;
        num2 = (int)((float)num2 / (float)(1 << digits));
        rem = num - (num2 << digits);

        ans[ans_index++] = arr[num2];
    }

    while (digits > 0) {
        if (digits == 4) {
            digits = 6;
            rem *= 4;
            ans[ans_index + 1] = '=';
        } else if (digits == 2) {
            digits = 6;
            rem *= 16;
            ans[ans_index + 1] = '=';
            ans[ans_index + 2] = '=';
        }

        int num = 0;
        for (int j = 1; j <= 6; j++) {
            num += rem & (1 << (digits - j));
        }
        num = (int)((float)rem / (float)(1 << (digits - 6)));
        digits -= 6;
        rem = rem - (num << digits);

        ans[ans_index++] = arr[num];
    }

    return ans;
}

int main(void) {
    // %s not good for outputing decoded value as bin may have \0 (early
    // termination)
    printf("%s\n", decode(encode("Many hands make light work.\n")));
    printf("%s\n", decode(encode("Man")));
    printf("%s\n", decode(encode("Ma")));
    printf("%s\n", decode(encode("M")));
    printf("%s\n", encode("Many hands make light work.\n"));
    printf("%s\n", encode("Man"));
    printf("%s\n", encode("Ma"));
    printf("%s\n", encode("M"));
    return 0;
}
