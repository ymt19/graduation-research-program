#include "log.h"
#include "transaction.h"
#include "batch.h"
#include "message.h"
#include "bytes.h"
#include <assert.h>
#include <stdio.h>

int main() {
    log_info_t *log_info = create_log_info("batch_log");

    int epoch = 0;
    char msg[BUFSIZ];
    char test_msg[BUFSIZ];
    int len;
    Tx *tx_a, *tx_b, *tx;
    Tx *test_tx_a, *test_tx_b;
    Batch *batch, *test_batch;

    // tx設定
    tx_a = create_tx();
    modify_to_write_tx(tx_a, 10, 20);
    set_client_info_to_tx(tx_a, 1, 2, 3);
    set_pointer_in_slave_to_tx(tx_a);
    tx_b = create_tx();
    modify_to_read_tx(tx_b, 30);
    set_client_info_to_tx(tx_b, 4, 5, 6);
    set_pointer_in_slave_to_tx(tx_b);

    // 仮バッチ作成:epoch:0
    batch = batch_create(0);
    batch_add_tx(batch, tx_a);
    batch_add_tx(batch, tx_b);
    len = create_batch_msg(batch, msg, BUFSIZ);
    append_log(log_info, msg, len, batch->epoch_id);
    batch_free(batch);

    // バッチ作成epoch:1
    batch = batch_create(1);
    batch_add_tx(batch, tx_a);
    batch_add_tx(batch, tx_b);
    len = create_batch_msg(batch, msg, BUFSIZ);
    append_log(log_info, msg, len, batch->epoch_id);


    // ログ読み取りepoch:1
    len = read_log(log_info, test_msg, 1);
    test_batch = batch_create(-1);
    get_info_from_batch_msg(test_msg, test_batch);
    printf("epochid%d txnum%d\n", test_batch->epoch_id, test_batch->tx_num);

    assert(batch->epoch_id == test_batch->epoch_id);
    assert(batch->tx_num == test_batch->tx_num);
    test_tx_a = batch->tx_set[0];
    test_tx_b = batch->tx_set[1];
    assert(tx_a->client_replica_id == test_tx_a->client_replica_id);
    assert(tx_a->client_id == test_tx_a->client_id);
    assert(tx_a->id == test_tx_a->id);
    assert(tx_a->key == test_tx_a->key);
    assert(tx_a->type == tx_a->type);
    assert(tx_a->value == tx_a->value);
    assert(tx_a == test_tx_a->pointer_in_slave);
    assert(tx_b->type == test_tx_b->type);
    assert(tx_b == test_tx_b->pointer_in_slave);
}