#include "message.h"
#include <assert.h>
#include <stdio.h>

void test_add_to_batch_message();
void test_batch_message();

int main() {
    test_add_to_batch_message();
    test_batch_message();
}

void test_add_to_batch_message() {
    char msg[BUFSIZ];
    Tx *tx_a, *tx_b;
    Tx *test_tx_a, *test_tx_b;

    tx_a = create_tx();
    modify_to_write_tx(tx_a, 10, 20);
    set_client_info_to_tx(tx_a, 1, 2, 3);
    tx_b = create_tx();
    modify_to_read_tx(tx_b, 30);
    set_client_info_to_tx(tx_b, 4, 5, 6);

    create_add_to_batch_msg(tx_a, msg);
    test_tx_a = create_tx();
    get_info_from_add_to_batch_msg(msg, test_tx_a);
    assert(tx_a->replica_id == test_tx_a->replica_id);
    assert(tx_a->client_id == test_tx_a->client_id);
    assert(tx_a->id == test_tx_a->id);
    assert(tx_a->key == test_tx_a->key);
    assert(tx_a->type == tx_a->type);
    assert(tx_a->value == tx_a->value);

    create_add_to_batch_msg(tx_b, msg);
    test_tx_b = create_tx();
    get_info_from_add_to_batch_msg(msg, test_tx_b);
    assert(tx_b->type == test_tx_b->type);
}

void test_batch_message() {
    int epoch = 10;
    char msg[BUFSIZ];
    Tx *tx_a, *tx_b, *tx;
    Tx *test_tx_a, *test_tx_b;
    Batch *batch, *test_batch;

    // tx設定
    tx_a = create_tx();
    modify_to_write_tx(tx_a, 10, 20);
    set_client_info_to_tx(tx_a, 1, 2, 3);
    tx_b = create_tx();
    modify_to_read_tx(tx_b, 30);
    set_client_info_to_tx(tx_b, 4, 5, 6);

    batch = batch_create(epoch);

    create_add_to_batch_msg(tx_a, msg);
    tx = create_tx();
    get_info_from_add_to_batch_msg(msg, tx);
    batch_add_tx(batch, tx);

    create_add_to_batch_msg(tx_b, msg);
    tx = create_tx();
    get_info_from_add_to_batch_msg(msg, tx);
    batch_add_tx(batch, tx);

    create_batch_msg(batch, msg, BUFSIZ);
    test_batch = batch_create(-1);
    get_info_from_batch_msg(msg, test_batch);

    assert(batch->epoch_id == test_batch->epoch_id);
    assert(batch->tx_num == test_batch->tx_num);
    test_tx_a = batch->tx_set[0];
    test_tx_b = batch->tx_set[1];
    assert(tx_a->client_id == test_tx_a->client_id);
    assert(tx_a->id == test_tx_a->id);
    assert(tx_a->key == test_tx_a->key);
    assert(tx_a->type == tx_a->type);
    assert(tx_a->value == tx_a->value);
    assert(tx_b->type == test_tx_b->type);
}