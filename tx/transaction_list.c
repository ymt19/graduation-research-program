#include "../tx/transaction_list.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

/**
 * @brief       バッチリストのメモリを確保
 */ 
tx_list_t *create_tx_list() {
    tx_list_t *tx_list = malloc(sizeof(tx_list_t));
    tx_list->head = NULL;
    tx_list->tail = NULL;
    // mutex初期化
    if (pthread_mutex_init(&tx_list->mutex, NULL) != 0) {
        // error()
        exit(1);
    }
    return tx_list;
}

/**
 * @brief       バッチリストのメモリを解放する
 */ 
void free_tx_list(tx_list_t *tx_list) {
    tx_list_node_t *node;
    while (tx_list->head != NULL) {
        node = tx_list->head;
        tx_list->head = tx_list->head->next;
        free(node);
        node = NULL;
    }
    pthread_mutex_destroy(&tx_list->mutex);
    free(tx_list);
    tx_list = NULL;
}

/**
 * @brief       バッチリストの末尾にノードを追加
 * @param       tx_list バッチリスト
 * @param       tx 追加するノードに保存するバッチ
 */
void add_node_tx_list(tx_list_t *tx_list, Tx *tx) {
    tx_list_node_t *new_node = malloc(sizeof(tx_list_node_t));
    new_node->tx = tx;
    new_node->next = NULL;

    pthread_mutex_lock(&tx_list->mutex);

    if (tx_list->tail == NULL) {         // リストが空
        tx_list->head = new_node;
        tx_list->tail = new_node;
    } else {
        tx_list->tail->next = new_node;
        tx_list->tail = new_node;
    }

    pthread_mutex_unlock(&tx_list->mutex);
}

/**
 * @brief       バッチリストの先頭のノードを削除
 * @param       tx_list バッチリスト
 * @return      削除したノードが保存していたバッチへのポインタ
 */
Tx *try_delete_node_tx_list(tx_list_t *tx_list) {
    tx_list_node_t *delete_node = NULL;             // 削除するノード
    Tx *tx;                                         // 返り値

    pthread_mutex_lock(&tx_list->mutex);

    if (tx_list->head == NULL) {                    // 要素数0
        tx = NULL;
    } else if (tx_list->head->next == NULL) {       // 要素数1
        delete_node = tx_list->head;
        tx_list->head = NULL;
        tx_list->tail = NULL;
        tx = delete_node->tx;
    } else {                                        // 要素数2以上
        delete_node = tx_list->head;
        tx_list->head = delete_node->next;
        tx = delete_node->tx;
    }

    pthread_mutex_unlock(&tx_list->mutex);

    free(delete_node);

    return tx;
}

/**
 * @brief       バッチリストが空かどうか判定
 * @param       tx_list 判定するバッチリスト
 * @return      1なら空、0なら空でない
 */
int tx_list_is_empty(tx_list_t *tx_list) {
    int ret;
    pthread_mutex_lock(&tx_list->mutex);
    if (tx_list->head == NULL) {
        ret = 1;
    } else {
        ret = 0;
    }
    pthread_mutex_unlock(&tx_list->mutex);
    return ret;
}