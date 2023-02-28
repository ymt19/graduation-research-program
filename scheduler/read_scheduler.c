#include "../utils/configuration.h"
#include "../scheduler/read_scheduler.h"
#include "../tx/transaction.h"
#include <stdio.h>
#include <stdlib.h>

void execute_read_tx(Storage *storage, Tx *tx);

void read_scheduler_startup(Config *config) {
    Tx *tx;
    Storage *storage = config->storage;

    printf("scheduler(read) startup!\n");

    while (config->system_finish_request == 0) {
        tx = NULL;
        tx = try_delete_node_tx_list(config->read_tx_buffer);

        if (tx != NULL) {
            execute_read_tx(storage, tx);
        }
    }
}

void execute_read_tx(Storage *storage, Tx *tx) {
    tx->value = read_from_storage(storage, tx->key);
    raise_tx_complete_flag(tx);
}