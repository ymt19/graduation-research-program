#include "bytes.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    int a = 1;
    int b = 2;
    int *a_p = &a;
    int *b_p = &b;
    int test_a, test_b, *test_a_p, *test_b_p;
    char msg[BUFSIZ];
    int offset;

    offset = 0;
    set_int_to_bytes(msg, offset, a);
    offset += sizeof(int);
    set_int_to_bytes(msg, offset, b);
    offset += sizeof(int);
    set_pointer_to_bytes(msg, offset, a_p);
    offset += sizeof(int *);
    set_pointer_to_bytes(msg, offset, b_p);
    offset += sizeof(int *);

    offset = 0;
    test_a = get_int_from_bytes(msg, offset);
    offset += sizeof(int);
    test_b = get_int_from_bytes(msg, offset);
    offset += sizeof(int);
    test_a_p = (int *)get_pointer_from_bytes(msg, offset);
    offset += sizeof(int *);
    test_b_p = (int *)get_pointer_from_bytes(msg, offset);

    assert(test_a == a);
    assert(test_b == b);
    assert(test_a_p == a_p);
    assert(test_b_p == b_p);
    assert(*test_a_p == a);
    assert(*test_b_p == b);
}