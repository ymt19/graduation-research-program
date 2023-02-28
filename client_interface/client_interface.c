#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../utils/configuration.h"
#include "../master/master_sequencer.h"
#include "../utils/message.h"
#include "../scheduler/read_scheduler.h"
#include "../network/tcp/tcp_client.h"
#include "../network/tcp/tcp_server.h"
#include "../utils/timer.h"
#include "../tx/transaction.h"
#include "../network/udp/udp_client.h"
#include "../network/udp/udp_server.h"

/**
 * @brief   client_thread()の引数
 */
struct client_thread_info_t {
    Config *config;

    // DBクライアントと接続しているソケットディスクリプタ
    int client_connected_sd;

    // マスターノードのシーケンサと通信するための情報
    // cl_info_t *cl_info;
    udp_cl_info_t *udp_cl_info;
};
typedef struct client_thread_info_t client_thread_info_t;

void *client_thread(client_thread_info_t *arg);
int request_read_tx(Tx *tx, Config *config);
int request_write_tx(Tx *tx, client_thread_info_t *arg);

void *client_interface_startup(Config *config) {
    int ret;
    int client_connected_sd;  // サーバ用ソケットディスクリプタ
    char errmsg[256];
    sv_info_t sv_info;
    int my_port;
    // cl_info_t cl_info_set[CLIENT_NUM];
    // int master_sequencer_port;
    // char master_sequencer_ipaddr[128];
    pthread_t worker[CLIENT_NUM];
    client_thread_info_t *arg;

    // 各clientと接続するソケットを用意
    my_port =
        get_port_slave_client_interface_from_client(config->my_replica_id);
    tcp_sv_socket_init(&sv_info, my_port, CLIENT_NUM, errmsg);

    // get_sequencer_ipaddr(0, master_sequencer_ipaddr);
    // master_sequencer_port =
    // get_port_master_sequencer_from_client_interface(); for (int i = 0; i <
    // CLIENT_NUM; i++) {
    //     tcp_cl_socket_init(&cl_info_set[i], master_sequencer_ipaddr,
    //     master_sequencer_port, errmsg); tcp_cl_connect(&cl_info_set[i],
    //     errmsg);
    // }

    // client interface起動完了
    printf("client interface start up!\n");

    // 終了要求メッセージ（フラグ付き）を受信するまで接続を受け付け
    for (int i = 0; i < CLIENT_NUM; i++) {
        client_connected_sd = tcp_sv_accept(&sv_info, errmsg);
        if (client_connected_sd < 0) {
            fprintf(stderr, "Error: %s\n", errmsg);
            exit(1);
        }

        arg = malloc(sizeof(client_thread_info_t));
        arg->client_connected_sd = client_connected_sd;
        arg->config = config;
        // arg->cl_info = &cl_info_set[i];

        // スレッド作成
        ret = pthread_create(&worker[i], NULL, (void *)client_thread,
                             (void *)arg);
        if (ret != 0) {
            // (todo) perror("pthread_create()");
            exit(1);
        }
    }

    udp_sv_info_t udp_sv_info;
    int finish_port = get_port_finish_from_client(config->my_replica_id);
    char recv_msg[256];
    int recv_msg_len;
    udp_sv_socket_init(&udp_sv_info, finish_port, 0, errmsg);
    while (1) {
        udp_sv_receive_msg(&udp_sv_info, recv_msg, 256, errmsg);
        if (strcmp(recv_msg, "finish") == 0) {
            printf("finish request\n");
            break;
        }
    }
    config->system_finish_request = 1;
    udp_sv_socket_deinit(&udp_sv_info);

    for (int i = 0; i < CLIENT_NUM; i++) {
        ret = pthread_join(worker[i], NULL);
        if (ret != 0) {
            // error処理
        }
    }

    // for (int i = 0; i < CLIENT_NUM; i++) {
    //     tcp_cl_socket_deinit(&cl_info_set[i]);
    // }
    tcp_sv_socket_deinit(&sv_info);
}

void *client_thread(client_thread_info_t *arg) {
    int ret;
    char recv_msg[BUFSIZ];
    int recv_msg_size = BUFSIZ;
    int recv_msg_len;
    char send_msg[BUFSIZ];
    int send_msg_size = BUFSIZ;
    int send_msg_len;
    char errmsg[BUFSIZ];
    udp_cl_info_t udp_cl_info;
    int my_udp_port;
    Tx *tx;

    udp_cl_socket_init(&udp_cl_info, 1, errmsg);

    arg->udp_cl_info = &udp_cl_info;

    tx = create_tx();
    while (arg->config->system_finish_request != 1) {
        recv_msg_len = tcp_sv_receive_msg(arg->client_connected_sd, recv_msg,
                                          recv_msg_size, errmsg);
        if (recv_msg_len < 0) {
            // error処理追加
            exit(1);
        }

        if (check_msg(recv_msg, REQUEST_TX_MESSAGE) == 1) {
            init_tx(tx);
            get_info_from_request_tx_msg(recv_msg, tx);
            print_tx_info(tx, "[client interface] get tx ");

            if (tx->type == WRITE_ONLY) {  // writeの場合
                request_write_tx(tx, arg);
            } else if (tx->type == READ_ONLY) {  // readの場合
                request_read_tx(tx, arg->config);
            } else if (tx->type == FINISH_REQUEST) {  // finishの場合
                printf("finish\n");
                // request_write_tx(tx, arg);
                // printf("finish ok\n");
                break;
            }
            // print_tx_info(tx, "[get tx]");

            // complete flagが立つまで待機
            while (tx->complete_flag == 0 &&
                   arg->config->system_finish_request == 0) {
            }
            if (arg->config->system_finish_request == 1) {
                break;
            }

            print_tx_info(tx, "[send tx]");
            send_msg_len = create_response_tx_msg(tx, send_msg);
            ret = tcp_sv_send_msg(arg->client_connected_sd, send_msg,
                                  send_msg_len, errmsg);
            if (ret < 0) {
                // error処理追加
                exit(1);
            }
        } else {
            if (check_msg(recv_msg, REQUEST_TERMINATE_MESSAGE)) {
                printf(
                    "[client interface] %d error message "
                    "REQUEST_TERMINATE_MESSAGE\n",
                    recv_msg_len);
            } else if (check_msg(recv_msg, RESPONSE_TERMINATE_MESSAGE)) {
                printf(
                    "[client interface] %d error message "
                    "RESPONSE_TERMINATE_MESSAGE\n",
                    recv_msg_len);
            } else if (check_msg(recv_msg, REQUEST_TX_MESSAGE)) {
                printf(
                    "[client interface] %d error message "
                    "REQUEST_TX_MESSAGE\n",
                    recv_msg_len);
            } else if (check_msg(recv_msg, RESPONSE_TX_MESSAGE)) {
                printf(
                    "[client interface] %d error message "
                    "RESPONSE_TX_MESSAGE\n",
                    recv_msg_len);
            } else if (check_msg(recv_msg, ADD_TO_BATCH_MESSAGE)) {
                printf(
                    "[client interface] %d error message "
                    "ADD_TO_BATCH_MESSAGE\n",
                    recv_msg_len);
            } else if (check_msg(recv_msg, BATCH_MESSAGE)) {
                printf(
                    "[client interface] %d error message "
                    "BATCH_MESSAGE\n",
                    recv_msg_len);
            } else if (check_msg(recv_msg, ACK_BATCH_MESSAGE)) {
                printf(
                    "[client interface] %d error message "
                    "ACK_BATCH_MESSAGE\n",
                    recv_msg_len);
            } else if (check_msg(recv_msg, REQUEST_BATCH_MESSAGE)) {
                printf(
                    "[client interface] %d error message "
                    "REQUEST_BATCH_MESSAGE\n",
                    recv_msg_len);
            }
        }
    }

    // tcp_cl_socket_deinit(&udp_cl_info);
    udp_cl_socket_deinit(&udp_cl_info);
    free_tx(tx);
    free(arg);
}

/**
 * @note    自ノードのスケジューラに送信
 */
int request_read_tx(Tx *tx, Config *config) {
    // スケジューラのバッチキュー(実行する)に追加
    add_node_tx_list(config->read_tx_buffer, tx);
}

/**
 * @note    マスタノードのシーケンサに送信
 *
 */
int request_write_tx(Tx *tx, client_thread_info_t *arg) {
    Config *config = arg->config;
    udp_cl_info_t *udp_cl_info = arg->udp_cl_info;
    int ret;
    char send_msg[BUFSIZ];
    int send_msg_size = BUFSIZ;
    int send_msg_len;
    char recv_msg[BUFSIZ];
    int recv_msg_size = BUFSIZ;
    int recv_msg_len;
    char errmsg[BUFSIZ];
    int replica_id = tx->replica_id;
    int client_id = tx->client_id;
    int tx_id = tx->id;
    int ack_replica_id;
    int ack_client_id;
    int ack_tx_id;
    int send_port;
    char send_ipaddr[128];
    int flag;
    double rto = get_rto(replica_id);
    double send_time;
    double recv_time;

    // ADD_TO_BATCHメッセージ作成
    send_msg_len = create_add_to_batch_msg(tx, send_msg);

    // 送信先IPアドレス・ポート
    send_port =
        get_port_master_sequencer_from_client_interface(replica_id, client_id);
    get_sequencer_ipaddr(0, send_ipaddr);

    // 実行待ちトランザクションに設定
    config->db_client_info_set[client_id].wait_for_exe = tx;

    // マスタノードのシーケンサに送信
    flag = 0;
    while (flag == 0) {
        if (config->system_finish_request == 1) {
            break;
        }
        udp_cl_send_msg(udp_cl_info, send_msg, send_msg_len, send_ipaddr,
                        send_port, errmsg);
        send_time = get_time();
        // send_logに書き込み
        if (config->system_finish_request == 0) {
            pthread_mutex_lock(&config->log_fp_mutex);
            fprintf(config->log_fp, "%d,%lf,%d,%d,%d,%d\n", 0, send_time,
                    send_msg_len, ADD_TO_BATCH_MESSAGE, 0, -1);
            // fflush(config->log_fp);
            pthread_mutex_unlock(&config->log_fp_mutex);
        }
        // if (config->system_finish_request == 0) {
        //     pthread_mutex_lock(&config->send_log_fp_mutex);
        //     fprintf(config->send_log_fp, "%lf,%d,%d,0\n", send_time,
        //             ADD_TO_BATCH_MESSAGE, send_msg_len);
        //     fflush(config->send_log_fp);
        //     pthread_mutex_unlock(&config->send_log_fp_mutex);
        // }

        // snprintf(errmsg, BUFSIZ, "ret:%d [client interface] SEND ADD_TO_BATCH
        // ",
        //          ret);
        // print_tx_info(tx, errmsg);

        while (config->system_finish_request == 0) {
            recv_msg_len = udp_cl_retrun_recv_msg(udp_cl_info, recv_msg,
                                                  recv_msg_size, errmsg);
            recv_time = get_time();

            if (recv_msg_len >= 0) {
                if (check_msg(recv_msg, ACK_ADD_MESSAGE) == 1) {
                    ack_replica_id = -1;
                    ack_client_id = -1;
                    ack_tx_id = -1;
                    get_info_from_ack_add_msg(recv_msg, &ack_replica_id,
                                              &ack_client_id, &ack_tx_id);

                    // recv_logに書き込み
                    if (config->system_finish_request == 0) {
                        pthread_mutex_lock(&config->log_fp_mutex);
                        fprintf(config->log_fp, "%d,%lf,%d,%d,%d,%d\n", 1,
                                recv_time, recv_msg_len, ACK_ADD_MESSAGE, 0,
                                -1);
                        // fflush(config->recv_log_fp);
                        pthread_mutex_unlock(&config->log_fp_mutex);
                    }
                    // if (config->system_finish_request == 0) {
                    //     pthread_mutex_lock(&config->recv_log_fp_mutex);
                    //     fprintf(config->recv_log_fp, "%lf,%d,%d\n",
                    //     recv_time,
                    //             ACK_ADD_MESSAGE, recv_msg_len);
                    //     fflush(config->recv_log_fp);
                    //     pthread_mutex_unlock(&config->recv_log_fp_mutex);
                    // }

                    if (ack_replica_id == replica_id &&
                        ack_client_id == client_id && ack_tx_id == tx_id) {
                        flag = 1;
                        break;
                    }
                }
            }

            if (get_time() >= send_time + rto) {
                printf("Overtime RTO rid%d,cid%d,tid%d\n", replica_id,
                       client_id, tx_id);
                break;
            }
        }
    }
    // ret = tcp_cl_send_msg(arg->cl_info, msg, len, errmsg);
    // snprintf(errmsg, BUFSIZ, "ret:%d [client interface] SEND MASTER: ", ret);
    // print_tx_info(tx, errmsg);
}