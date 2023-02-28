#include "../utils/storage.h"
#include <stdlib.h>

/**
 * @brief       ストレージのメモリ領域を確保
 * @param       storage_size レコード数
 * @return      確保したメモリ
 */
Storage *new_storage(short storage_size) {
    Storage *storage;

    storage = malloc(sizeof(Storage));
    
    storage->storage_size = storage_size;
    storage->records = malloc(sizeof(Record) * storage_size);
    for (int i = 0; i < storage_size; i++) {
        storage->records[i].value = 0;
        if (pthread_mutex_init(&(storage->records[i].record_mutex), NULL) != 0) {
            // error()
            exit(1);
        }
    }

    return storage;
}

void free_storage(Storage *storage) {
    for (int i = 0; i < storage->storage_size; i++) {
        pthread_mutex_destroy(&(storage->records[i].record_mutex));
    }
    free(storage->records);
    free(storage);
}

/**
 * @brief       ストレージの特定のレコードの値を得る（スレッドセーフ）
 * @param[in]   storage
 * @param[in]   key レコードを指定するキー [0]-[STORAGE_SIZE]
 * @return      値
 */
int read_from_storage(Storage *storage, short key) {
    int value;
    if (pthread_mutex_lock(&(storage->records[key].record_mutex)) != 0) {
        // perror()
        exit(1);
    }
    value = storage->records[key].value;
    if (pthread_mutex_unlock(&(storage->records[key].record_mutex)) != 0) {
        // perror()
        exit(1);
    }
    return value;
}

/**
 * @brief       ストレージの特定のレコードの値を変更する（スレッドセーフ）
 * @param[in]   storage
 * @param[in]   key レコードを指定するキー [0]-[STORAGE_SIZE]
 * @param[in]   value
 */
void write_to_storage(Storage *storage, short key, short value) {
    if (pthread_mutex_lock(&(storage->records[key].record_mutex)) != 0) {
        // perror()
        exit(1);
    }
    storage->records[key].value++;
    if (pthread_mutex_unlock(&(storage->records[key].record_mutex)) != 0) {
        // perror()
        exit(1);
    }
}