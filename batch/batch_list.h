#pragma once

#include "../batch/batch.h"
#include <pthread.h>

/**
 * @struct      バッチリストノード情報
 */
struct batch_list_node_type {
    Batch *batch;
    struct batch_list_node_type *next;
};
typedef struct batch_list_node_type batch_list_node_t;

/**
 * @struct      バッチリスト情報
 */
struct batch_list_type {
    batch_list_node_t *head;
    batch_list_node_t *tail;
    pthread_mutex_t mutex;
};
typedef struct batch_list_type batch_list_t;

batch_list_t *create_batch_list();
void free_batch_list(batch_list_t *batch_list);
void add_node_batch_list(batch_list_t *batch_list, Batch *batch);
Batch *try_delete_node_batch_list(batch_list_t *batch_list);