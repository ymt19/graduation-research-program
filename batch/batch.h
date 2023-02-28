#pragma once

#include "../tx/transaction.h"

#define MAX_BATCH_SIZE          90

/**
 * @brief   マスタで確定されるWriteセットのバッチ
 */
struct Batch {
    // エポックID
    short epoch_id;

    // 格納されているTXの個数
    short tx_num;

    // トランザクション列（Txのポインタの配列）
    Tx *tx_set[MAX_BATCH_SIZE];
};
typedef struct Batch Batch;

Batch* batch_create(short epoch_id);
void batch_free(Batch *batch);
// void batch_set_info(Batch *batch, int epoch_id);
// void batch_set_expected_tx_num(Batch *batch, int expected_tx_num);
int batch_add_tx(Batch *batch, Tx *tx);
// void batch_set_tx(Batch *batch, Tx tx, int pos);
// int batch_is_complete(Batch *batch);