#pragma once

#include <pthread.h>

/**
 * ストレージに配置されるレコード
 */
struct Record {
    short value;
    pthread_mutex_t record_mutex;
};
typedef struct Record Record;

/**
 * メモリ内に格納されるストレージ
 */
struct Storage {
    short storage_size;
    Record *records;
};
typedef struct Storage Storage;

Storage *new_storage(short storage_size);
void free_storage(Storage *storage);
int read_from_storage(Storage *storage, short key);
void write_to_storage(Storage *storage, short key, short value);