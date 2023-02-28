#include "../utils/configuration.h"
#include "../utils/log.h"
#include "../batch/batch.h"
#include "../utils/message.h"
#include "../utils/bytes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

static void read_all(int fd, char *bytes, unsigned int goal_size);
static void write_all(int fd, char *bytes, unsigned int goal_size);

/**
 * @brief Create a log info object
 * 
 * @param filename
 * @return log_info_t* 
 * 
 * 未完成
 */
log_info_t *create_log_info(char *filename) {
    log_info_t *log_info = malloc(sizeof(log_info_t));

    strcpy(log_info->filename, filename);
    pthread_mutex_init(&log_info->mutex, NULL);

    // ファイルの初期化
    remove(log_info->filename);

    return log_info;
}

/**
 * @brief 
 * 
 * @param log_info 
 */
void free_log_info(log_info_t *log_info) {
    pthread_mutex_destroy(&log_info->mutex);
    free(log_info);
}

/**
 * @brief ログを追加
 * 
 * @param log_info 
 * @param batch 
 */
void append_log(log_info_t *log_info, char *msg, int len, int epoch_id) {
    int fd;
    int offset = BATCH_LOG_SIZE * epoch_id;
    char bytes[BATCH_LOG_SIZE];

    // log fileに書き込むデータ生成
    set_int_to_bytes(bytes, BATCH_LOG_DATA_SIZE_OFFSET, len);
    memcpy(bytes + BATCH_LOG_DATA_OFFSET, msg, len);

    pthread_mutex_lock(&log_info->mutex);       // lock
    // printf("lock\n");
    if ((fd = open(log_info->filename, O_WRONLY | O_CREAT, 0777)) == -1) {
        perror("open");
        exit(1);
    }
    if (lseek(fd, offset, SEEK_SET) == -1) {
        perror("lseek");
        exit(1);
    }
    write_all(fd, bytes, BATCH_LOG_SIZE);
    fsync(fd);
    close(fd);
    // printf("unlock\n");
    pthread_mutex_unlock(&log_info->mutex);     // unlock
}

/**
 * @brief 
 * 
 * @param log_info 
 * @param msg 
 * @param epoch_id 
 * @return int 
 */
int read_log(log_info_t *log_info, char *msg, int epoch_id) {
    int fd;
    int offset = BATCH_LOG_SIZE * epoch_id;
    int len;
    char bytes[BATCH_LOG_SIZE];

    pthread_mutex_lock(&log_info->mutex);
    // printf("lock\n");
    if ((fd = open(log_info->filename, O_RDONLY)) == -1) {
        perror("open");
        exit(1);
    }
    if (lseek(fd, offset, SEEK_SET) == -1) {
        perror("lseek");
        exit(1);
    }
    read_all(fd, bytes, BATCH_LOG_SIZE);
    close(fd);
    // printf("unlock\n");
    pthread_mutex_unlock(&log_info->mutex);

    len = get_int_from_bytes(bytes, BATCH_LOG_DATA_SIZE_OFFSET);
    memcpy(msg, bytes + BATCH_LOG_DATA_OFFSET, len);
    // return len;
    return BATCH_LOG_SIZE;
}

/**
 * 指定する範囲のデータをすべて読み込むことを保証する
 * read(2)のラッパー関数
 * 
 * fd       : ファイルディスクリプタ
 * bytes    : readするデータ
 * goal_size: readするデータサイズ
 */
static void read_all(int fd, char *bytes, unsigned int goal_size) {
    int rd_size;            // 1回のread(2)で読み込まれたデータサイズ
    int total_rd_size = 0;  // 読み込まれたデータサイズの合計

    while (total_rd_size != goal_size) {
        if ((rd_size = read(fd, bytes + total_rd_size, goal_size - total_rd_size)) == -1) {
            perror("read");
            exit(1);
        }
        // printf("rdsize%d\n", rd_size);
        // printf("total read size:%d\n", total_rd_size);
        total_rd_size += rd_size;
    }
}

/**
 * 指定する範囲のデータをすべて書き込むことを保証する
 * write(2)のラッパー関数
 * 
 * fd       : ファイルディスクリプタ
 * bytes    : writeするデータ
 * goal_size: writeするデータサイズ
 */
static void write_all(int fd, char *bytes, unsigned int goal_size) {
    int wr_size;             // 1回のwrite(2)で書き込まれたデータサイズ
    int total_wr_size = 0;   // 書き込まれたデータサイズの合計

    while (total_wr_size != goal_size) {
        if ((wr_size = write(fd, bytes + total_wr_size, goal_size - total_wr_size)) == -1) {
            perror("write");
            exit(1);
        }

        total_wr_size += wr_size;
    }
}