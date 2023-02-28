#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../scheduler/write_scheduler.h"
#include "../batch/batch.h"
#include "../batch/batch_list.h"
#include "../utils/configuration.h"
#include "../utils/message.h"
#include "../utils/storage.h"
#include "../tx/transaction.h"

void execute_batch(Config *config, Batch *batch, short epoch_id);

void write_scheduler_startup(Config *config) {
    int epoch;
    Batch *batch;

    // スケジューラ起動完了
    printf("scheduler(write) start up!\n");

    epoch = 0;
    while (config->system_finish_request == 0) {
        batch = NULL;
        batch = try_delete_node_batch_list(config->write_tx_buffer);

        // バッチの実行
        if (batch != NULL) {
            // printf("[scheduler] get batch epoch:%d batch_epoch:%d
            // tx_num:%d\n", epoch, batch->epoch_id, batch->tx_num);
            execute_batch(config, batch, epoch);
            epoch++;
        }
    }
}

/**
 * @brief       バッチに格納されているWriteトランザクションを実行
 * @param       storage
 * @param       batch
 * @param       epoch_id
 */
void execute_batch(Config *config, Batch *batch, short epoch_id) {
    Tx *tx;
    short tx_c_id, tx_r_id;
    Tx *wait_for_tx;
    for (int i = 0; i < batch->tx_num; i++) {
        tx = batch->tx_set[i];
        tx_c_id = tx->client_id;
        tx_r_id = tx->replica_id;

        set_assigned_epoch_id_to_tx(tx, epoch_id);
        print_tx_info(tx, "[executer]");
        write_to_storage(config->storage, tx->key, tx->value);
        free_tx(tx);

        if (tx_r_id == config->my_replica_id) {
            wait_for_tx = config->db_client_info_set[tx_c_id].wait_for_exe;
            set_assigned_epoch_id_to_tx(wait_for_tx, epoch_id);
            raise_tx_complete_flag(wait_for_tx);
        }
    }

    // 実行したバッチのメモリを解放
    batch_free(batch);
}