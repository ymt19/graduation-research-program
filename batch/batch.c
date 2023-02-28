#include "../batch/batch.h"
#include "../tx/transaction.h"
#include <string.h>
#include <stdlib.h>

Batch* batch_create(short epoch_id) {
    Batch *batch = (Batch*)malloc(sizeof(Batch));
    batch->epoch_id = epoch_id;
    batch->tx_num = 0;
    return batch;
}

void batch_free(Batch *batch) {
    free(batch);
}

// void batch_set_info(Batch *batch, int epoch_id) {
//     batch->epoch_id = epoch_id;
// }

/**
 * @note
 * スケジューラが使用
 */
// void batch_set_expected_tx_num(Batch *batch, int expected_tx_num) {
//     batch->expected_tx_num = expected_tx_num;
// }

/**
 * @note
 * シーケンサが使用
 */
int batch_add_tx(Batch *batch, Tx *tx) {
    if (batch->tx_num < MAX_BATCH_SIZE) {
        batch->tx_set[batch->tx_num] = tx;
        batch->tx_num++;
        return 1;
    } else {
        return 0;
    }
}

// /**
//  * @note
//  * スケジューラが使用
//  */
// void batch_set_tx(Batch *batch, Tx tx, int pos) {
//     tx_cpy(&(batch->tx_set[pos]), tx);
//     if (pos == batch->last) {
//         batch->last++;
//     }

//     // batch->tx_numとbatch->expected_tx_numを比較するbatch_is_complete()を
//     // 他のスレッドが用いるため、最後にbatch->tx_numを変更する
//     batch->tx_num++;
// }


/**
 * @return 完了していたら0、そうでないなら-1
 * @note
 * スケジューラが使用
 */
// int batch_is_complete(Batch *batch) {
//     if (batch->tx_num == batch->expected_tx_num) {
//         return 0;
//     } else {
//         return -1;
//     }
// }
