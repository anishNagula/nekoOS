#include "utils.h"

int my_strcmp(const char* a, const char* b, int length) {
    for (int i = 0; i < length; i++) {
        if (a[i] != b[i]) {
            return a[i] - b[i];
        }
        if (a[i] == '\0' || b[i] == '\0') {
            return 0;
        }
    }
    return 0;
}

void trim_whitespace(char* str, int* start, int* length) {
    int s = *start;
    int len = *length;

    while (str[s] == ' ' && len > 0) {
        s++;
        len--;
    }

    while (len > 0 && str[s + len - 1] == ' ') {
        len--;
    }

    *start = s;
    *length = len;
}
