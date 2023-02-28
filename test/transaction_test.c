#include "transaction.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    transaction tx = new_transaction(1, READ_WRITE_SET);

    add_read_op(&tx, 0);
    add_read_op(&tx, 1);
    add_write_op(&tx, 2, 2);
    add_write_op(&tx, 3, 3);

    int i;
    operation op;

    op = tx.operations[i];
    assert(op.type == READ);
    assert(op.key == i);
    assert(op.value == -1);
    i++;

    op = tx.operations[i];
    assert(op.type == READ);
    assert(op.key == 1);
    assert(op.value == -1);
    i++;

    op = tx.operations[i];
    assert(op.type == WRITE);
    assert(op.key == i);
    assert(op.value == i);
    i++;

    op = tx.operations[i];
    assert(op.type == WRITE);
    assert(op.key == i);
    assert(op.value == i);
    i++;

    return 0;
}