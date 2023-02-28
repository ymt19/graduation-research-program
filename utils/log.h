#pragma once

#include <pthread.h>

// 1つのlogにおけるオフセット
#define BATCH_LOG_DATA_SIZE_OFFSET      0
#define BATCH_LOG_DATA_OFFSET           BATCH_LOG_DATA_SIZE_OFFSET+sizeof(int)

// log1つ分のサイズ（固定長）
#define BATCH_LOG_SIZE                  1024

struct log_info_t {
    char filename[100];
    pthread_mutex_t mutex;
    // short batch_msg_size[1000];
};
typedef struct log_info_t log_info_t;

log_info_t *create_log_info(char *filename);
void free_log_info(log_info_t *log_info);
void append_log(log_info_t *log_info, char *msg, int len, int epoch_id);
int read_log(log_info_t *log_info, char *msg, int epoch_id);