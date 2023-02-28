#pragma once

#include <pthread.h>
#include "../tx/transaction.h"

/**
 * @struct      トランザクションリストノード情報
 */
struct tx_list_node_type {
    Tx *tx;
    struct tx_list_node_type *next;
};
typedef struct tx_list_node_type tx_list_node_t;

/**
 * @struct      バッチリスト情報
 */
struct tx_list_type {
    tx_list_node_t *head;
    tx_list_node_t *tail;
    pthread_mutex_t mutex;
};
typedef struct tx_list_type tx_list_t;

tx_list_t *create_tx_list();
void free_tx_list(tx_list_t *tx_list);
void add_node_tx_list(tx_list_t *tx_list, Tx *tx);
Tx *try_delete_node_tx_list(tx_list_t *tx_list);
// int tx_list_is_empty(tx_list_t *tx_list);