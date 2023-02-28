#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "../utils/configuration.h"
#include "../utils/message.h"
#include "../network/tcp/tcp_client.h"
#include "../utils/timer.h"
#include "../tx/transaction.h"

#define FILENAME "result/result.csv"
pthread_mutex_t result_csv_mutex;

struct client_thread_info_t {
    short replica_id;
    short client_id;

    // 0-100
    int write_rate;

    int seed;

    FILE *fp;

    double client_start_time;
    int tx_num;
};
typedef struct client_thread_info_t client_thread_info_t;

int get_random(int min, int max, int *seed);
void *client_thread(client_thread_info_t *client_thread_info);

double global_end_time_set[REPLICA_NUM][CLIENT_NUM];

int main(int argc, char *argv[]) {
    FILE *fp;
    int ret;
    int tx_num;
    client_thread_info_t *client_thread_info;
    pthread_t **client_thread_worker;
    double client_start_time;

    if (argc != 2) {
        fprintf(stdout, "<tx num>\n");
        exit(1);
    }
    tx_num = atoi(argv[1]);

    // csvファイル
    pthread_mutex_init(&result_csv_mutex, NULL);
    fp = fopen(FILENAME, "w");
    if (fp == NULL) {
        return -1;
    }

    // マスタを除く各レプリカにthread_num分のクライアントを配置
    client_thread_worker = malloc(sizeof(pthread_t *) * REPLICA_NUM);
    for (int i = 0; i < REPLICA_NUM; i++) {
        client_thread_worker[i] = malloc(sizeof(pthread_t) * CLIENT_NUM);
    }

    // スレッド生成
    client_start_time = get_time();
    for (int replica_id = 1; replica_id < REPLICA_NUM; replica_id++) {
        for (int client_id = 0; client_id < CLIENT_NUM; client_id++) {
            client_thread_info = malloc(sizeof(client_thread_info_t));
            client_thread_info->replica_id = replica_id;
            client_thread_info->client_id = client_id;
            client_thread_info->write_rate = 100;
            client_thread_info->seed = replica_id * 100 + client_id;
            client_thread_info->fp = fp;
            client_thread_info->client_start_time = client_start_time;
            client_thread_info->tx_num = tx_num;
            global_end_time_set[replica_id][client_id] = -1;

            ret = pthread_create(&client_thread_worker[replica_id][client_id],
                                 NULL, (void *)client_thread,
                                 (void *)client_thread_info);
            if (ret != 0) {
                // error
            }
        }
    }

    // スレッドjoin
    for (int replica_id = 1; replica_id < REPLICA_NUM; replica_id++) {
        for (int client_id = 0; client_id < CLIENT_NUM; client_id++) {
            ret =
                pthread_join(client_thread_worker[replica_id][client_id], NULL);
            if (ret != 0) {
                // error
            }
        }
    }

    if (pthread_mutex_lock(&result_csv_mutex) != 0) {
        // perror()
        exit(1);
    }
    for (int replica_id = 1; replica_id < REPLICA_NUM; replica_id++) {
        for (int client_id = 0; client_id < CLIENT_NUM; client_id++) {
            fprintf(fp, "%d,%d,%lf,%lf,%lf\n", replica_id, client_id,
                    client_start_time, global_end_time_set[replica_id][client_id],
                    global_end_time_set[replica_id][client_id] - client_start_time);
            fflush(fp);
        }
    }
    if (pthread_mutex_unlock(&result_csv_mutex) != 0) {
        // perror()
        exit(1);
    }

    fclose(fp);
    pthread_mutex_destroy(&result_csv_mutex);

    for (int i = 0; i < REPLICA_NUM; i++) {
        free(client_thread_worker[i]);
    }
    free(client_thread_worker);
}

void *client_thread(client_thread_info_t *client_thread_info) {
    int type, key;
    cl_info_t cl_info;
    int len;
    char msg[BUFSIZ];
    char errmsg[256];
    Tx *send_tx, *recv_tx;
    int write_num = 0, read_num = 0;
    double send_time, recv_time;
    int replica_id = client_thread_info->replica_id;
    int client_id = client_thread_info->client_id;
    FILE *fp = client_thread_info->fp;
    double client_start_time = client_thread_info->client_start_time;
    double client_end_time = -1;
    int tx_num = client_thread_info->tx_num;
    char ipaddr[BUFSIZ];
    int port;

    // 自ノードのclinet interfaceに接続
    get_client_interface_ipaddr(replica_id, ipaddr);
    port = get_port_slave_client_interface_from_client(replica_id);
    tcp_cl_socket_init(&cl_info, ipaddr, port, errmsg);
    tcp_cl_connect(&cl_info, errmsg);

    // トランザクション要求開始
    send_tx = create_tx();
    recv_tx = create_tx();
    for (int tx_id = 0; tx_id < tx_num; tx_id++) {
        init_tx(send_tx);
        init_tx(recv_tx);
        set_client_info_to_tx(send_tx, client_thread_info->replica_id,
                              client_thread_info->client_id, tx_id);

        type = get_random(0, 99, &(client_thread_info->seed));
        key = get_random(0, STORAGE_SIZE - 1, &(client_thread_info->seed));
        if (type < client_thread_info->write_rate) {  // write
            modify_to_write_tx(send_tx, key, -1);
            write_num++;
        } else {  // read
            modify_to_read_tx(send_tx, key);
            // read_num++;
        }

        // 送信
        len = create_request_tx_msg(send_tx, msg);
        send_time = get_time();
        tcp_cl_send_msg(&cl_info, msg, len, errmsg);
        // print_tx_info(send_tx, "[send]");

        // 受信
        len = tcp_cl_receive_msg(&cl_info, msg, BUFSIZ, errmsg);
        recv_time = get_time();
        client_end_time = recv_time;
        get_info_from_response_tx_msg(msg, recv_tx);
        print_tx_info(recv_tx, "[get]");

        if (pthread_mutex_lock(&result_csv_mutex) != 0) {
            // perror()
            exit(1);
        }
        fprintf(fp, "%lf,%lf,%lf,%d,%d,%d,%d\n", send_time, recv_time,
                recv_time - send_time, replica_id, client_id, tx_id,
                recv_tx->assigned_epoch_id);
        fflush(fp);
        if (pthread_mutex_unlock(&result_csv_mutex) != 0) {
            // perror()
            exit(1);
        }
    }

    // 終了要求
    modify_finish_request(send_tx);
    len = create_request_tx_msg(send_tx, msg);
    tcp_cl_send_msg(&cl_info, msg, len, errmsg);

    free_tx(send_tx);
    free_tx(recv_tx);
    tcp_cl_socket_deinit(&cl_info);

    free(client_thread_info);

    printf("replica:%d client:%d write:%d, read:%d endtime:%lf\n",
            replica_id, client_id, write_num, read_num, client_end_time);

    global_end_time_set[replica_id][client_id] = client_end_time;
}

/**
 * @brief       指定範囲内のランダムな整数値を取得
 * @param       min 最小値
 * @param       max 最大値
 * @return      ランダム値
 */
int get_random(int min, int max, int *seed) {
    return min + rand_r(seed) % (max - min + 1);
}