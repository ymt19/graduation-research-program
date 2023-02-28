// トランザクションとメッセージ間の変換
// transaction -> message
// message -> transaction
#include "../utils/bytes.h"
#include <stdio.h>
#include <string.h>

void set_int_to_bytes(char *bytes, int offset, int value) {
    memcpy(bytes + offset, &value, sizeof(int));
}

int get_int_from_bytes(char *bytes, int offset) {
    int value;
    memcpy(&value, bytes + offset, sizeof(int));
    return value;
}

void set_short_int_to_bytes(char *bytes, int offset, short value) {
    memcpy(bytes + offset, &value, sizeof(short));
}

short get_short_int_from_bytes(char *bytes, int offset) {
    short value;
    memcpy(&value, bytes + offset, sizeof(short));
    return value;
}

void set_char_to_bytes(char *bytes, int offset, char value) {
    memcpy(bytes + offset, &value, sizeof(char));
}

char get_char_from_bytes(char *bytes, int offset) {
    char value;
    memcpy(&value, bytes + offset, sizeof(char));
    return value;
}

void set_pointer_to_bytes(char *bytes, int offset, void *p) {
    memcpy(bytes + offset, &p, sizeof(void *));
}

void *get_pointer_from_bytes(char *bytes, int offset) {
    void *p;
    memcpy(&p, bytes + offset, sizeof(void *));
    return p;
}