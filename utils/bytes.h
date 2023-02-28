#pragma once

#include "../tx/transaction.h"

void set_int_to_bytes(char *bytes, int offset, int value);
int get_int_from_bytes(char *bytes, int offset);
void set_short_int_to_bytes(char *bytes, int offset, short value);
short get_short_int_from_bytes(char *bytes, int offset);
void set_char_to_bytes(char *bytes, int offset, char value);
char get_char_from_bytes(char *bytes, int offset);
void set_pointer_to_bytes(char *bytes, int offset, void *p);
void *get_pointer_from_bytes(char *bytes, int offset);