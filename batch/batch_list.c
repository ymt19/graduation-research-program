#include "../batch/batch_list.h"
#include <stdlib.h>
#include <pthread.h>

/**
 * @brief       バッチリストのメモリを確保
 */ 
batch_list_t *create_batch_list() {
    batch_list_t *batch_list = malloc(sizeof(batch_list_t));
    batch_list->head = NULL;
    batch_list->tail = NULL;
    if (pthread_mutex_init(&batch_list->mutex, NULL) != 0) {
        // error()
        exit(1);
    }
    return batch_list;
}

/**
 * @brief       バッチリストのメモリを解放する
 */ 
void free_batch_list(batch_list_t *batch_list) {
    batch_list_node_t *node;
    while (batch_list->head != NULL) {
        node = batch_list->head;
        batch_list->head = batch_list->head->next;
        free(node);
    }
    pthread_mutex_destroy(&batch_list->mutex);
    free(batch_list);
}

/**
 * @brief       バッチリストの末尾にノードを追加
 * @param       batch_list バッチリスト
 * @param       batch 追加するノードに保存するバッチ
 */
void add_node_batch_list(batch_list_t *batch_list, Batch *batch) {
    batch_list_node_t *new_node = malloc(sizeof(batch_list_node_t));
    new_node->batch = batch;
    new_node->next = NULL;

    pthread_mutex_lock(&batch_list->mutex);

    if (batch_list->tail == NULL) {         // リストが空
        batch_list->head = new_node;
        batch_list->tail = new_node;
    } else {
        batch_list->tail->next = new_node;
        batch_list->tail = new_node;
    }

    pthread_mutex_unlock(&batch_list->mutex);
}

/**
 * @brief       バッチリストの先頭のノードを削除
 * @param       batch_list バッチリスト
 * @return      削除したノードが保存していたバッチへのポインタ、空ならNULL
 */
Batch *try_delete_node_batch_list(batch_list_t *batch_list) {
    batch_list_node_t *delete_node = NULL;          // 削除するノード
    Batch *batch = NULL;                            // 返り値

    pthread_mutex_lock(&batch_list->mutex);

    if (batch_list->head == NULL) {                 // 既にリストが空 
        batch = NULL;
    } else if (batch_list->head->next == NULL) {    // 削除するとリストが空
        delete_node = batch_list->head;
        batch_list->head = NULL;
        batch_list->tail = NULL;
        batch = delete_node->batch;
    } else {
        delete_node = batch_list->head;
        batch_list->head = delete_node->next;
        batch = delete_node->batch;
    }

    pthread_mutex_unlock(&batch_list->mutex);

    free(delete_node);
    return batch;
}

/**
 * @brief       バッチリストが空かどうか判定
 * @param       batch_list 判定するバッチリスト
 * @return      1なら空、0なら空でない
 */
int is_empty_batch_list(batch_list_t *batch_list) {
    int ret;

    pthread_mutex_lock(&batch_list->mutex);
    if (batch_list->head == NULL) {
        ret = 1;
    } else {
        ret = 0;
    }
    pthread_mutex_unlock(&batch_list->mutex);

    return ret;
}