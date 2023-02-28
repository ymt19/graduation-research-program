#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../master/master_sequencer.h"
#include "../batch/batch.h"
#include "../utils/configuration.h"
#include "../utils/log.h"
#include "../utils/message.h"
#include "../utils/storage.h"
#include "../network/tcp/tcp_client.h"
#include "../network/tcp/tcp_server.h"
#include "../utils/timer.h"
#include "../tx/transaction.h"
#include "../network/udp/udp_client.h"
#include "../network/udp/udp_server.h"
#include "../scheduler/write_scheduler.h"

/**
 * @brief   recieve_tx_connectionの引数
 */
struct recieve_tx_connection_info_t {
    Config *config;
    int replica_id;
    int client_id;
    // int connected_sd;
};
typedef struct recieve_tx_connection_info_t recieve_tx_connection_info_t;

/**
 * @brief   response_connectionの引数
 */
struct send_batch_connection_info_t {
    Config *config;
    int connection_replica_id;
};
typedef struct send_batch_connection_info_t send_batch_connection_info_t;

/**
 * 現在のエポックで作成しているバッチ
 */
Batch *batch;
pthread_mutex_t batch_variable_mutex;

/**
 * 確定したバッチの中で最も新しいバッチのエポック番号
 * （重要）batch logを更新後にこの値を更新
 */
int determined_epoch_id;

void *recieve_tx_connection(recieve_tx_connection_info_t *arg);
void *send_batch_connection(send_batch_connection_info_t *arg);
void *proposed_method_send_batch_connection(send_batch_connection_info_t *arg);
void *conventional_method_send_batch_connection(
    send_batch_connection_info_t *arg);

/**
 * @brief   シーケンサが現在受け付けているバッチにWriteトランザクションを追加
 * @param   tx 追加するトランザクション
 * @return  成功なら1、失敗なら0（バッチサイズの上限超過のため）
 * @note    マルチスレッド対応
 */
int sequencer_add_write_tx_to_batch(Tx *tx) {
    int ret = 0;

    if (pthread_mutex_lock(&batch_variable_mutex) != 0) {
        // perror()
        exit(1);
    }
    if (batch->tx_num < MAX_BATCH_SIZE) {
        ret = batch_add_tx(batch, tx);
    }
    if (pthread_mutex_unlock(&batch_variable_mutex) != 0) {
        // perror()
        exit(1);
    }

    return ret;
}

/**
 * @brief   （マスタノード用）シーケンサmainスレッド
 * @param   void_config main関数で設定した情報
 * @note
 * この関数は、複数のクライアントスレッドからwrite処理を受信し（master_sequencer_add_write()を
 * 経由して）、現在の関数がバッチを作成。
 * この関数が同ノードのスケジューラにそのバッチを送信する。
 * この関数は、全スレーブノードと通信を行う複数の送受信スレッドを作成。
 */
void *master_sequencer_startup(Config *config) {
    int ret;
    int total_client_num = CLIENT_NUM * (REPLICA_NUM - 1);
    // sv_info_t sv_info;
    // int sv_port;
    // int connected_sd;
    // pthread_t *recieve_tx_connection_thread_set;
    pthread_t recieve_tx_connection_thread_set[REPLICA_NUM][CLIENT_NUM];
    pthread_t *send_batch_connection_thread_set;
    recieve_tx_connection_info_t *recieve_tx_connection_info;
    send_batch_connection_info_t *send_batch_connection_info;
    int len;
    double epoch_start_time;
    char batch_msg[BATCH_LOG_SIZE];
    int batch_msg_size = BATCH_LOG_SIZE;
    char errmsg[BUFSIZ];
    Batch *rev_batch;

    // 初期のバッチを作成
    batch = batch_create(0);
    determined_epoch_id = -1;
    // mutex初期化
    pthread_mutex_init(&batch_variable_mutex, NULL);

    for (int replica_id = 1; replica_id < REPLICA_NUM; replica_id++) {
        for (int client_id = 0; client_id < CLIENT_NUM; client_id++) {
            recieve_tx_connection_info =
                malloc(sizeof(recieve_tx_connection_info_t));
            recieve_tx_connection_info->config = config;
            recieve_tx_connection_info->replica_id = replica_id;
            recieve_tx_connection_info->client_id = client_id;

            // スレッド作成
            ret = pthread_create(
                &recieve_tx_connection_thread_set[replica_id][client_id], NULL,
                (void *)recieve_tx_connection,
                (void *)recieve_tx_connection_info);
            if (ret != 0) {
                // (todo) perror("pthread_create()");
                exit(1);
            }
        }
    }

    // send batch connectionスレッドをレプリカ数分だけ開始
    send_batch_connection_thread_set = malloc(sizeof(pthread_t) * REPLICA_NUM);
    for (int r_id = 1; r_id < REPLICA_NUM; r_id++) {
        send_batch_connection_info =
            malloc(sizeof(send_batch_connection_info_t));
        send_batch_connection_info->config = config;
        send_batch_connection_info->connection_replica_id = r_id;

        if (config->exp == tcp_method) {  // tcp手法
            // ret = pthread_create(&send_batch_connection_thread_set[r_id],
            // NULL,
            //                      (void *)send_batch_connection,
            //                      (void *)send_batch_connection_info);
            // if (ret != 0) {
            //     // (todo) perror("pthread_create()");
            //     exit(1);
            // }
        } else if (config->exp == proposed_method) {  // 提案手法
            ret = pthread_create(&send_batch_connection_thread_set[r_id], NULL,
                                 (void *)proposed_method_send_batch_connection,
                                 (void *)send_batch_connection_info);
            if (ret != 0) {
                // (todo) perror("pthread_create()");
                exit(1);
            }
        } else if (config->exp == conventional_method) {  // 従来手法
            ret = pthread_create(
                &send_batch_connection_thread_set[r_id], NULL,
                (void *)conventional_method_send_batch_connection,
                (void *)send_batch_connection_info);
            if (ret != 0) {
                // (todo) perror("pthread_create()");
                exit(1);
            }
        }
    }

    // シーケンサ起動完了
    printf("sequencer start up!\n");

    // システム起動時間セット
    printf("start creating batches!\n");

    // clientが起動するまで待つ
    while (config->system_finish_request != 0) {
    }

    // エポックごとにbatchを作成
    int current_epoch_id;
    epoch_start_time = get_time();
    while (config->system_finish_request == 0) {
        current_epoch_id = determined_epoch_id + 1;

        // エポック開始時間がシステム稼働時間を超過している場合
        if (config->system_finish_request == 1) {
            break;
        }

        // エポック期間だけクライアントからの要求を受け付けバッチを決定
        // （別スレッドのclient_interface.c内でbatchにtxを追加）
        while (get_time() < epoch_start_time + config->epoch_duration) {
        }
        // sleep(config->epoch_duration);

        // 決定したバッチをスケジューラのバッチキューに追加
        pthread_mutex_lock(&batch_variable_mutex);  // lock
        rev_batch = batch;
        batch = batch_create(current_epoch_id + 1);
        pthread_mutex_unlock(&batch_variable_mutex);  // unlock

        // バッチを入れ替えた直後に時間測定開始
        epoch_start_time = get_time();

        // batch logに確定バッジを追加
        len = create_batch_msg(rev_batch, batch_msg, batch_msg_size);
        append_log(config->batch_log, batch_msg, len, rev_batch->epoch_id);
        determined_epoch_id++;

        // スケジューラに確定バッジを送信
        add_node_batch_list(config->write_tx_buffer, rev_batch);
        printf("Confirm batch epoch:%d tx_num:%d len:%d\n", current_epoch_id,
               rev_batch->tx_num, len);

        // logにバッチ生成時間を書き込み
        pthread_mutex_lock(&config->log_fp_mutex);
        fprintf(config->log_fp, "%d,%lf,%d,%d,%d,%d\n", 2, epoch_start_time, -1,
                -1, -1, current_epoch_id);
        // fflush(config->recv_log_fp);
        pthread_mutex_unlock(&config->log_fp_mutex);
    }

    /* 終了処理開始 */
    // request connection threadのjoin
    for (int replica_id = 1; replica_id < REPLICA_NUM; replica_id++) {
        for (int client_id = 0; client_id < CLIENT_NUM; client_id++) {
            ret = pthread_join(
                recieve_tx_connection_thread_set[replica_id][client_id], NULL);
            if (ret != 0) {
                // error処理
            }
        }
    }

    // response connection threadのjoin
    for (int i = 1; i < REPLICA_NUM; i++) {
        ret = pthread_join(send_batch_connection_thread_set[i], NULL);
        if (ret != 0) {
            // error処理
        }
    }

    pthread_mutex_destroy(&batch_variable_mutex);
    // tcp_sv_socket_deinit(&sv_info);
    // free(recieve_tx_connection_thread_set);
    free(send_batch_connection_thread_set);
}

/**
 * @brief       スレーブノードのclient interfaceからリクエストを受信
 * @note
 * サーバの役割を担う。
 * 1. 要求トランザクションを受信
 * 3. システム終了要請を受信
 */
void *recieve_tx_connection(recieve_tx_connection_info_t *arg) {
    Config *config = arg->config;
    // int connected_sd = arg->connected_sd;
    int replica_id = arg->replica_id;
    int client_id = arg->client_id;
    int recieved_tx_id = -1;
    udp_sv_info_t udp_sv_info;
    int my_port;
    double recv_time;
    char recv_msg[BUFSIZ];
    int recv_msg_size = BUFSIZ;
    int recv_msg_len;
    double send_time;
    char send_msg[BUFSIZ];
    int send_msg_size = BUFSIZ;
    int send_msg_len;
    char errmsg[BUFSIZ];
    Tx *tx;

    my_port =
        get_port_master_sequencer_from_client_interface(replica_id, client_id);
    udp_sv_socket_init(&udp_sv_info, my_port, 0, errmsg);

    while (config->system_finish_request != 0) {
    }

    // トランザクションを受信したらバッチに追加する
    while (config->system_finish_request == 0) {
        recv_msg_len =
            udp_sv_receive_msg(&udp_sv_info, recv_msg, recv_msg_size, errmsg);
        recv_time = get_time();
        if (check_msg(recv_msg, ADD_TO_BATCH_MESSAGE) == 1) {
            tx = NULL;
            tx = create_tx();
            get_info_from_add_to_batch_msg(recv_msg, tx);

            // recv_logに書き込み
            if (config->system_finish_request == 0) {
                pthread_mutex_lock(&config->log_fp_mutex);
                fprintf(config->log_fp, "%d,%lf,%d,%d,%d,%d\n", 1, recv_time,
                        recv_msg_len, ADD_TO_BATCH_MESSAGE, replica_id, -1);
                // fflush(config->recv_log_fp);
                pthread_mutex_unlock(&config->log_fp_mutex);

                // pthread_mutex_lock(&config->recv_log_fp_mutex);
                // fprintf(config->recv_log_fp, "%lf,%d,%d\n", recv_time,
                //         ADD_TO_BATCH_MESSAGE, recv_msg_len);
                // fflush(config->recv_log_fp);
                // pthread_mutex_unlock(&config->recv_log_fp_mutex);
            }
            // printf("RECV TX rid%d,cid%d,txid%d,len%d\n", tx->replica_id,
            //        tx->client_id, tx->id, recv_msg_len);

            send_msg_len = create_ack_add_msg(tx->replica_id, tx->client_id,
                                              tx->id, send_msg);
            udp_sv_return_send_msg(&udp_sv_info, send_msg, send_msg_len,
                                   errmsg);
            send_time = get_time();
            // send_logに書き込み
            if (config->system_finish_request == 0) {
                pthread_mutex_lock(&config->log_fp_mutex);
                fprintf(config->log_fp, "%d,%lf,%d,%d,%d,%d\n", 0, send_time,
                        send_msg_len, ACK_ADD_MESSAGE, replica_id, -1);
                // fflush(config->send_log_fp);
                pthread_mutex_unlock(&config->log_fp_mutex);

                // pthread_mutex_lock(&config->send_log_fp_mutex);
                // fprintf(config->send_log_fp, "%lf,%d,%d,0\n", send_time,
                //         ACK_ADD_MESSAGE, send_msg_len);
                // fflush(config->send_log_fp);
                // pthread_mutex_unlock(&config->send_log_fp_mutex);
            }
            // printf("SEND ACK ADD rid%d,cid%d,txid%d\n", tx->replica_id,
            //        tx->client_id, tx->id);

            if (tx->id <= recieved_tx_id) {
                continue;
            } else {
                recieved_tx_id = tx->id;
            }

            if (tx->type == WRITE_ONLY) {
                while (sequencer_add_write_tx_to_batch(tx) == 0) {
                }
            } else if (tx->type == FINISH_REQUEST) {
                break;
            } else {
                printf("[master sequencer] recv error tx\n");
                break;
            }
        } else {
            // printf("[master sequencer] get error message.\n");
        }
    }

    udp_sv_socket_deinit(&udp_sv_info);
    free(arg);
}

/**
 * @brief （提案手法）スレーブノードのsequencerにレスポンスを送信
 *
 * @param arg
 * @return void*
 */
void *proposed_method_send_batch_connection(send_batch_connection_info_t *arg) {
    Config *config = arg->config;
    int connection_replica_id = arg->connection_replica_id;
    udp_cl_info_t cl_info;
    udp_sv_info_t sv_info;
    int next_epoch_id = 0;  // 次に送信するバッジのエポックID
    int send_epoch_id;      // 送信中エポックID
    char send_msg[BATCH_LOG_SIZE];
    int send_msg_size = BATCH_LOG_SIZE;
    int send_msg_len;
    int success_send_msg_len;
    double recv_time;
    double first_send_time;
    double send_time;
    char recv_msg[BUFSIZ];
    int recv_msg_size = BATCH_LOG_SIZE;
    int recv_msg_len;
    char errmsg[256];
    short my_port;
    short send_port;
    char send_ipaddr[128];
    int get_msg_batch_epoch_id;  // 受信したメッセージに含まれるバッチエポックID
    int get_msg_replica_id;  // 受信したメッセージを送信したレプリカ
    double rto = get_rto(connection_replica_id);

    // 自身のポート番号
    my_port =
        get_port_master_sequencer_from_slave_sequencer(connection_replica_id);
    printf("replica:%d my port:%d\n", connection_replica_id, my_port);
    // 送信先のポート番号とIPアドレス
    send_port = get_port_slave_sequencer(connection_replica_id);
    get_sequencer_ipaddr(connection_replica_id, send_ipaddr);

    udp_cl_socket_init(&cl_info, 0, errmsg);
    udp_sv_socket_init(&sv_info, my_port, 1, errmsg);

    while (config->system_finish_request != 0) {
    }

    // 作成されたバッチを確認したら送信する
    while (config->system_finish_request == 0) {
        if (determined_epoch_id >= next_epoch_id) {
            send_epoch_id = next_epoch_id;
            send_msg_len =
                read_log(arg->config->batch_log, send_msg, send_epoch_id);

            int flag = 0;  // 現在のバッチデータが送信完了しているかを表すフラグ
            first_send_time = get_time();
            while (flag == 0) {
                if (config->system_finish_request == 1) {
                    break;
                }
                // バッチデータ送信
                success_send_msg_len =
                    udp_cl_send_msg(&cl_info, send_msg, send_msg_len,
                                    send_ipaddr, send_port, errmsg);
                send_time = get_time();
                // send_logに書き込み
                if (config->system_finish_request == 0) {
                    pthread_mutex_lock(&config->log_fp_mutex);
                    fprintf(config->log_fp, "%d,%lf,%d,%d,%d,%d\n", 0,
                            send_time, success_send_msg_len, BATCH_MESSAGE,
                            connection_replica_id, send_epoch_id);
                    // fflush(config->send_log_fp);
                    pthread_mutex_unlock(&config->log_fp_mutex);

                    // pthread_mutex_lock(&config->send_log_fp_mutex);
                    // fprintf(config->send_log_fp, "%lf,%d,%d,0\n", send_time,
                    //         BATCH_MESSAGE, success_send_msg_len);
                    // fflush(config->send_log_fp);
                    // pthread_mutex_unlock(&config->send_log_fp_mutex);
                }
                // printf(
                //     "SEND BATCH_MESSAGE epoch_id:%d replica_id:%d len:%d "
                //     "success:%d\n",
                //     send_epoch_id, connection_replica_id, send_msg_len,
                //     success_send_msg_len);

                // ACK受信まで待機
                while (1) {
                    // ACKのノンブロッキング受信
                    recv_msg_len = udp_sv_receive_msg(&sv_info, recv_msg,
                                                      recv_msg_size, errmsg);
                    recv_time = get_time();
                    if (recv_msg_len >= 0) {
                        if (check_msg(recv_msg, ACK_BATCH_MESSAGE) == 1) {
                            get_info_from_ack_batch_msg(
                                recv_msg, &get_msg_replica_id,
                                &get_msg_batch_epoch_id);

                            // printf(
                            //     "RECV ACK_MESSAGE epoch_id:%d replica_id:
                            //     %d\n", get_msg_batch_epoch_id,
                            //     get_msg_replica_id);
                            if (get_msg_batch_epoch_id >= send_epoch_id) {
                                // recv_logに書き込み
                                if (config->system_finish_request == 0) {
                                    pthread_mutex_lock(&config->log_fp_mutex);
                                    fprintf(config->log_fp,
                                            "%d,%lf,%d,%d,%d,%d,%lf\n", 1,
                                            recv_time, recv_msg_len,
                                            ACK_BATCH_MESSAGE,
                                            get_msg_replica_id,
                                            get_msg_batch_epoch_id,
                                            recv_time - first_send_time);
                                    // fflush(config->recv_log_fp);
                                    pthread_mutex_unlock(&config->log_fp_mutex);

                                    // pthread_mutex_lock(
                                    //     &config->recv_log_fp_mutex);
                                    // fprintf(config->recv_log_fp,
                                    //         "%lf,%d,%d,%lf\n", recv_time,
                                    //         ACK_BATCH_MESSAGE, recv_msg_len,
                                    //         recv_time - first_send_time);
                                    // fflush(config->recv_log_fp);
                                    // pthread_mutex_unlock(
                                    //     &config->recv_log_fp_mutex);
                                }
                                flag = 1;
                                next_epoch_id = get_msg_batch_epoch_id + 1;
                                break;
                            }
                        }
                    }

                    // タイムアウト経過していた場合に再送
                    if (get_time() >= send_time + rto) {
                        printf("Overtime RTO epoch_id:%d\n", send_epoch_id);
                        break;
                    }
                }
            }
        }
    }

    udp_cl_socket_deinit(&cl_info);
    udp_sv_socket_deinit(&sv_info);
    free(arg);
}

/**
 * @brief （従来手法）スレーブノードのsequencerにレスポンスを送信
 *
 * @param arg
 * @return void*
 */
void *conventional_method_send_batch_connection(
    send_batch_connection_info_t *arg) {
    Config *config = arg->config;
    int connection_replica_id = arg->connection_replica_id;
    udp_cl_info_t cl_info;
    udp_sv_info_t sv_info;
    int next_epoch_id = 0;  // 次に送信するバッジのエポックID
    int send_epoch_id;      // 送信中エポックID
    double send_time;
    double first_send_time;
    double recv_time;
    char send_msg[BATCH_LOG_SIZE];
    int send_msg_size = BATCH_LOG_SIZE;
    int send_msg_len;
    int success_send_msg_len;
    char recv_msg[BUFSIZ];
    int recv_msg_size = BATCH_LOG_SIZE;
    int recv_msg_len;
    char errmsg[256];
    int my_port;
    short send_port;
    char send_ipaddr[128];
    int get_msg_batch_epoch_id;  // 受信したメッセージに含まれるバッチエポックID
    int get_msg_replica_id;  // 受信したメッセージを送信したレプリカ
    double rto = get_rto(connection_replica_id);

    // 自身のポート番号
    my_port =
        get_port_master_sequencer_from_slave_sequencer(connection_replica_id);
    printf("replica:%d my port:%d\n", connection_replica_id, my_port);
    // 送信先のポート番号とIPアドレス
    send_port = get_port_slave_sequencer(connection_replica_id);
    get_sequencer_ipaddr(connection_replica_id, send_ipaddr);

    udp_cl_socket_init(&cl_info, 0, errmsg);
    udp_sv_socket_init(&sv_info, my_port, 1, errmsg);

    while (config->system_finish_request != 0) {
    }

    // 作成されたバッチを確認したら送信する
    while (config->system_finish_request == 0) {
        if (determined_epoch_id >= next_epoch_id) {
            send_epoch_id = next_epoch_id;
            send_msg_len =
                read_log(arg->config->batch_log, send_msg, send_epoch_id);

            int flag = 0;  // 現在のバッチデータが送信完了しているかを表すフラグ
            first_send_time = get_time();
            while (flag == 0) {
                if (config->system_finish_request == 1) {
                    break;
                }
                // バッチデータ送信
                success_send_msg_len =
                    udp_cl_send_msg(&cl_info, send_msg, send_msg_len,
                                    send_ipaddr, send_port, errmsg);
                send_time = get_time();
                // send_logに書き込み
                if (config->system_finish_request == 0) {
                    pthread_mutex_lock(&config->log_fp_mutex);
                    fprintf(config->log_fp, "%d,%lf,%d,%d,%d,%d\n", 0,
                            send_time, success_send_msg_len, BATCH_MESSAGE,
                            connection_replica_id, send_epoch_id);
                    // fflush(config->send_log_fp);
                    pthread_mutex_unlock(&config->log_fp_mutex);

                    // pthread_mutex_lock(&config->send_log_fp_mutex);
                    // fprintf(config->send_log_fp, "%lf,%d,%d,0\n", send_time,
                    //         BATCH_MESSAGE, success_send_msg_len);
                    // fflush(config->send_log_fp);
                    // pthread_mutex_unlock(&config->send_log_fp_mutex);
                }
                // printf(
                //     "SEND BATCH_MESSAGE epoch_id:%d replica_id:%d len:%d "
                //     "success:%d\n",
                //     send_epoch_id, connection_replica_id, send_msg_len,
                //     success_send_msg_len);

                // ACK受信まで待機
                while (1) {
                    // ACKのノンブロッキング受信
                    recv_msg_len = udp_sv_receive_msg(&sv_info, recv_msg,
                                                      recv_msg_size, errmsg);
                    recv_time = get_time();
                    if (recv_msg_len < 0) {
                        if (errno == EAGAIN) {
                            // printf("yet len:%d\n", recv_msg_len);
                        }
                    } else {
                        if (check_msg(recv_msg, ACK_BATCH_MESSAGE) == 1) {
                            get_info_from_ack_batch_msg(
                                recv_msg, &get_msg_replica_id,
                                &get_msg_batch_epoch_id);
                            // printf(
                            //     "RECV ACK_MESSAGE epoch_id:%d
                            //     replica_id:%d\n", get_msg_batch_epoch_id,
                            //     get_msg_replica_id);
                            if (get_msg_batch_epoch_id == send_epoch_id) {
                                // recv_logに書き込み
                                if (config->system_finish_request == 0) {
                                    pthread_mutex_lock(&config->log_fp_mutex);
                                    fprintf(config->log_fp,
                                            "%d,%lf,%d,%d,%d,%d,%lf\n", 1,
                                            recv_time, recv_msg_len,
                                            ACK_BATCH_MESSAGE,
                                            get_msg_replica_id,
                                            get_msg_batch_epoch_id,
                                            recv_time - first_send_time);
                                    // fflush(config->recv_log_fp);
                                    pthread_mutex_unlock(&config->log_fp_mutex);

                                    // pthread_mutex_lock(
                                    //     &config->recv_log_fp_mutex);
                                    // fprintf(config->recv_log_fp,
                                    //         "%lf,%d,%d,%lf\n", recv_time,
                                    //         ACK_BATCH_MESSAGE, recv_msg_len,
                                    //         recv_time - first_send_time);
                                    // fflush(config->recv_log_fp);
                                    // pthread_mutex_unlock(
                                    //     &config->recv_log_fp_mutex);
                                }
                                flag = 1;
                                next_epoch_id = get_msg_batch_epoch_id + 1;
                                break;
                            }
                        }
                    }

                    // タイムアウト経過していた場合に再送
                    if (get_time() >= send_time + rto) {
                        printf("Overtime RTO epoch_id:%d\n", send_epoch_id);
                        break;
                    }

                    // sleep(MASTER_RECV_DURATION);
                }
            }
        }
    }

    udp_cl_socket_deinit(&cl_info);
    udp_sv_socket_deinit(&sv_info);
    free(arg);
}