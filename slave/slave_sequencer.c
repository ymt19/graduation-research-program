#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../utils/configuration.h"
#include "../utils/log.h"
#include "../utils/message.h"
#include "../network/tcp/tcp_server.h"
#include "../utils/timer.h"
#include "../network/udp/udp_client.h"
#include "../network/udp/udp_server.h"

void recieve_batch_connection(Config *config);  // tcp手法
void proposed_method_recieve_batch_connection(Config *config);
void conventional_method_recieve_batch_connection(Config *config);

/**
 * @brief       スレーブノードのsequencer
 */
void *slave_sequencer(Config *config) {
    if (config->exp == tcp_method) {
        // recieve_batch_connection(config);
    } else if (config->exp == proposed_method) {  // 提案
        proposed_method_recieve_batch_connection(config);
    } else if (config->exp == conventional_method) {  // 従来
        conventional_method_recieve_batch_connection(config);
    }
}

/**
 * @brief       （提案手法）マスタノードのsequencerからバッチを受信
 */
void proposed_method_recieve_batch_connection(Config *config) {
    int my_replica_id = config->my_replica_id;
    udp_sv_info_t sv_info;
    udp_cl_info_t cl_info;
    char errmsg[256];
    char send_msg[BATCH_LOG_SIZE];
    int send_msg_size = BATCH_LOG_SIZE;
    int send_msg_len;
    char recv_msg[BATCH_LOG_SIZE];
    int recv_msg_size = BATCH_LOG_SIZE;
    int recv_msg_len;
    short my_port;
    short send_port;
    char send_ipaddr[128];
    Batch *batch;
    int next_batch_epoch_id = 0;
    int get_msg_batch_epoch_id;  // メッセージに含まれるバッチエポックID
    int get_msg_replica_id;  // メッセージを送信したレプリカ
    int my_region_id = get_region(my_replica_id);
    double send_time;
    double recv_time;

    my_port = get_port_slave_sequencer(my_replica_id);

    udp_cl_socket_init(&cl_info, 0, errmsg);
    udp_sv_socket_init(&sv_info, my_port, 0, errmsg);

    // 受信 and 実行バッファに追加
    while (config->system_finish_request == 0) {
        recv_msg_len =
            udp_sv_receive_msg(&sv_info, recv_msg, recv_msg_size, errmsg);
        recv_time = get_time();
        if (check_msg(recv_msg, BATCH_MESSAGE)) {  // BATCH_MESSAGE

            /** recv_msgから情報取得 **/
            get_msg_batch_epoch_id = get_epoch_id_from_batch_meg(recv_msg);
            // recv_logに書き込み
            if (config->system_finish_request == 0) {
                pthread_mutex_lock(&config->log_fp_mutex);
                fprintf(config->log_fp, "%d,%lf,%d,%d,%d,%d\n", 1, recv_time,
                        recv_msg_len, BATCH_MESSAGE, 0, get_msg_batch_epoch_id);
                // fflush(config->recv_log_fp);
                pthread_mutex_unlock(&config->log_fp_mutex);

                // pthread_mutex_lock(&config->recv_log_fp_mutex);
                // fprintf(config->recv_log_fp, "%lf,%d,%d\n", recv_time,
                //         BATCH_MESSAGE, recv_msg_len);
                // fflush(config->recv_log_fp);
                // pthread_mutex_unlock(&config->recv_log_fp_mutex);
            }
            // printf("RECV BATCH_MESSAGE epoch_id:%d len:%d\n",
            //        get_msg_batch_epoch_id, recv_msg_len);
            /**************************/

            if (get_msg_batch_epoch_id == next_batch_epoch_id) {
                next_batch_epoch_id++;

                /** log追加 **/
                append_log(config->batch_log, recv_msg, recv_msg_len,
                           get_msg_batch_epoch_id);
                /*************/

                // batchオブジェクト作成
                batch = NULL;
                batch = batch_create(get_msg_batch_epoch_id);
                get_info_from_batch_msg(recv_msg, batch);
                add_node_batch_list(config->write_tx_buffer, batch);

                /** masterと同一regionのslaveにACK送信 **/
                // ACKメッセージ作成
                send_msg_len = create_ack_batch_msg(
                    my_replica_id, get_msg_batch_epoch_id, send_msg);

                // masterにACK送信
                send_port = get_port_master_sequencer_from_slave_sequencer(
                    my_replica_id);
                get_sequencer_ipaddr(0, send_ipaddr);
                udp_cl_send_msg(&cl_info, send_msg, send_msg_len, send_ipaddr,
                                send_port, errmsg);
                send_time = get_time();

                // send_logに書き込み
                if (config->system_finish_request == 0) {
                    pthread_mutex_lock(&config->log_fp_mutex);
                    fprintf(config->log_fp, "%d,%lf,%d,%d,%d,%d\n", 0,
                            send_time, send_msg_len, ACK_BATCH_MESSAGE, 0,
                            get_msg_batch_epoch_id);
                    // fflush(config->send_log_fp);
                    pthread_mutex_unlock(&config->log_fp_mutex);

                    // pthread_mutex_lock(&config->send_log_fp_mutex);
                    // fprintf(config->send_log_fp, "%lf,%d,%d,0\n", send_time,
                    //         ACK_BATCH_MESSAGE, send_msg_len);
                    // fflush(config->send_log_fp);
                    // pthread_mutex_unlock(&config->send_log_fp_mutex);
                }

                // 同一リージョンのslaveにACK送信
                for (int id = 1; id < REPLICA_NUM; id++) {
                    if (id != my_replica_id && get_region(id) == my_region_id) {
                        send_port = get_port_slave_sequencer(id);
                        get_within_region_slave_ipaddr(id, send_ipaddr);
                        udp_cl_send_msg(&cl_info, send_msg, send_msg_len,
                                        send_ipaddr, send_port, errmsg);
                        send_time = get_time();

                        // send_logに書き込み(スレーブ間通信)
                        if (config->system_finish_request == 0) {
                            pthread_mutex_lock(&config->log_fp_mutex);
                            fprintf(config->log_fp, "%d,%lf,%d,%d,%d,%d\n", 0,
                                    send_time, send_msg_len, ACK_BATCH_MESSAGE,
                                    id, get_msg_batch_epoch_id);
                            // fflush(config->send_log_fp);
                            pthread_mutex_unlock(&config->log_fp_mutex);

                            // pthread_mutex_lock(&config->send_log_fp_mutex);
                            // fprintf(config->send_log_fp, "%lf,%d,%d,1\n",
                            //         send_time, ACK_BATCH_MESSAGE,
                            //         send_msg_len);
                            // fflush(config->send_log_fp);
                            // pthread_mutex_unlock(&config->send_log_fp_mutex);
                        }
                    }
                }
                /******************************************/
            } else if (get_msg_batch_epoch_id < next_batch_epoch_id) {
                /** masterと同一リージョンのsalveにACK送信 **/
                // ACKメッセージ作成
                send_msg_len = create_ack_batch_msg(
                    my_replica_id, get_msg_batch_epoch_id, send_msg);

                // masterにACK送信
                send_port = get_port_master_sequencer_from_slave_sequencer(
                    my_replica_id);
                get_sequencer_ipaddr(0, send_ipaddr);
                udp_cl_send_msg(&cl_info, send_msg, send_msg_len, send_ipaddr,
                                send_port, errmsg);
                send_time = get_time();

                // send_logに書き込み
                if (config->system_finish_request == 0) {
                    pthread_mutex_lock(&config->log_fp_mutex);
                    fprintf(config->log_fp, "%d,%lf,%d,%d,%d,%d\n", 0,
                            send_time, send_msg_len, ACK_BATCH_MESSAGE, 0,
                            get_msg_batch_epoch_id);
                    // fflush(config->send_log_fp);
                    pthread_mutex_unlock(&config->log_fp_mutex);

                    // pthread_mutex_lock(&config->send_log_fp_mutex);
                    // fprintf(config->send_log_fp, "%lf,%d,%d,0\n", send_time,
                    //         ACK_BATCH_MESSAGE, send_msg_len);
                    // fflush(config->send_log_fp);
                    // pthread_mutex_unlock(&config->send_log_fp_mutex);
                }
            } else if (get_msg_batch_epoch_id > next_batch_epoch_id) {
                // 来ないはず
                assert(0);
            }

        } else if (check_msg(recv_msg, ACK_BATCH_MESSAGE)) {
            /** recv_msgから情報取得 **/
            get_info_from_ack_batch_msg(recv_msg, &get_msg_replica_id,
                                        &get_msg_batch_epoch_id);
            // recv_logに書き込み
            if (config->system_finish_request == 0) {
                pthread_mutex_lock(&config->log_fp_mutex);
                fprintf(config->log_fp, "%d,%lf,%d,%d,%d,%d\n", 1, recv_time,
                        recv_msg_len, ACK_BATCH_MESSAGE, get_msg_replica_id,
                        get_msg_batch_epoch_id);
                // fflush(config->recv_log_fp);
                pthread_mutex_unlock(&config->log_fp_mutex);

                // pthread_mutex_lock(&config->recv_log_fp_mutex);
                // fprintf(config->recv_log_fp, "%lf,%d,%d\n", recv_time,
                //         ACK_BATCH_MESSAGE, recv_msg_len);
                // fflush(config->recv_log_fp);
                // pthread_mutex_unlock(&config->recv_log_fp_mutex);
            }
            // printf("RECV ACK_MESSAGE get_epoch_id:%d replica:%d\n",
            //        get_msg_batch_epoch_id, get_msg_replica_id);
            /**************************/

            if (get_msg_batch_epoch_id >= next_batch_epoch_id) {
                /** request(batch=next_batch_epoch)を送信 **/
                // REQUEST_BATCHメッセージ作成
                send_msg_len = create_request_batch_msg(
                    my_replica_id, next_batch_epoch_id, send_msg);
                // ACKメッセージ送信元レプリカにREQUEST_BATCHメッセージ送信
                send_port = get_port_slave_sequencer(get_msg_replica_id);
                get_within_region_slave_ipaddr(get_msg_replica_id, send_ipaddr);
                udp_cl_send_msg(&cl_info, send_msg, send_msg_len, send_ipaddr,
                                send_port, errmsg);
                send_time = get_time();
                /*******************************************/

                // send_logに書き込み（スレーブ間通信）
                if (config->system_finish_request == 0) {
                    pthread_mutex_lock(&config->log_fp_mutex);
                    fprintf(config->log_fp, "%d,%lf,%d,%d,%d,%d\n", 0,
                            send_time, send_msg_len, REQUEST_BATCH_MESSAGE,
                            get_msg_replica_id, next_batch_epoch_id);
                    // fflush(config->send_log_fp);
                    pthread_mutex_unlock(&config->log_fp_mutex);

                    // pthread_mutex_lock(&config->send_log_fp_mutex);
                    // fprintf(config->send_log_fp, "%lf,%d,%d,1\n", send_time,
                    //         REQUEST_BATCH_MESSAGE, send_msg_len);
                    // fflush(config->send_log_fp);
                    // pthread_mutex_unlock(&config->send_log_fp_mutex);
                }
                // printf(
                //     "SEND REQUEST_BATCH_MESSAGE BATCH req_epoch_id:%d "
                //     "send_replica:%d\n",
                //     next_batch_epoch_id, get_msg_replica_id);
            }

        } else if (check_msg(recv_msg, REQUEST_BATCH_MESSAGE)) {
            /** recv_msgから情報取得 **/
            get_info_from_request_batch_message(recv_msg, &get_msg_replica_id,
                                                &get_msg_batch_epoch_id);
            /**************************/
            // recv_logに書き込み
            if (config->system_finish_request == 0) {
                pthread_mutex_lock(&config->log_fp_mutex);
                fprintf(config->log_fp, "%d,%lf,%d,%d,%d,%d\n", 1, recv_time,
                        recv_msg_len, REQUEST_BATCH_MESSAGE, get_msg_replica_id,
                        get_msg_batch_epoch_id);
                // fflush(config->recv_log_fp);
                pthread_mutex_unlock(&config->log_fp_mutex);

                // pthread_mutex_lock(&config->recv_log_fp_mutex);
                // fprintf(config->recv_log_fp, "%lf,%d,%d\n", recv_time,
                //         REQUEST_BATCH_MESSAGE, recv_msg_len);
                // fflush(config->recv_log_fp);
                // pthread_mutex_unlock(&config->recv_log_fp_mutex);
            }
            // printf("RECV REQUEST_BATCH_MESSAGE get_epochid:%d
            // replica:%d\n",
            //        get_msg_batch_epoch_id, get_msg_replica_id);

            /** logからget_msg_batch_epoch_idのsned_msgを生成 **/
            send_msg_len =
                read_log(config->batch_log, send_msg, get_msg_batch_epoch_id);
            /***************************************************/

            /** send_msgを送信 **/
            send_port = get_port_slave_sequencer(get_msg_replica_id);
            get_within_region_slave_ipaddr(get_msg_replica_id, send_ipaddr);
            udp_cl_send_msg(&cl_info, send_msg, send_msg_len, send_ipaddr,
                            send_port, errmsg);
            send_time = get_time();
            /********************/

            // send_logに書き込み(スレーブ間通信)
            if (config->system_finish_request == 0) {
                pthread_mutex_lock(&config->log_fp_mutex);
                fprintf(config->log_fp, "%d,%lf,%d,%d,%d,%d\n", 0, send_time,
                        send_msg_len, BATCH_MESSAGE, get_msg_replica_id,
                        get_msg_batch_epoch_id);
                // fflush(config->send_log_fp);
                pthread_mutex_unlock(&config->log_fp_mutex);

                // pthread_mutex_lock(&config->send_log_fp_mutex);
                // fprintf(config->send_log_fp, "%lf,%d,%d,1\n", send_time,
                //         BATCH_MESSAGE, send_msg_len);
                // fflush(config->send_log_fp);
                // pthread_mutex_unlock(&config->send_log_fp_mutex);
            }
            // printf("SEND BATCH_MESSAGE epoch_id:%d send_replica:%d\n",
            //        get_msg_batch_epoch_id, get_msg_replica_id);
        }
    }

    udp_cl_socket_deinit(&cl_info);
    udp_sv_socket_deinit(&sv_info);
}

/**
 * @brief       （従来手法）マスタノードのsequencerからバッチを受信
 */
void conventional_method_recieve_batch_connection(Config *config) {
    int my_replica_id = config->my_replica_id;
    udp_sv_info_t sv_info;
    udp_cl_info_t cl_info;
    char errmsg[256];
    char send_msg[BATCH_LOG_SIZE];
    int send_msg_size = BATCH_LOG_SIZE;
    int send_msg_len;
    char recv_msg[BATCH_LOG_SIZE];
    int recv_msg_size = BATCH_LOG_SIZE;
    int recv_msg_len;
    short my_port;
    short send_port;
    char send_ipaddr[128];
    Batch *batch;
    int next_batch_epoch_id = 0;
    int get_msg_batch_epoch_id;
    double send_time, recv_time;

    my_port = get_port_slave_sequencer(my_replica_id);

    udp_cl_socket_init(&cl_info, 0, errmsg);
    udp_sv_socket_init(&sv_info, my_port, 0, errmsg);

    // 受信 and 実行バッファに追加
    while (config->system_finish_request == 0) {
        recv_msg_len =
            udp_sv_receive_msg(&sv_info, recv_msg, recv_msg_size, errmsg);
        recv_time = get_time();
        if (check_msg(recv_msg, BATCH_MESSAGE)) {  // BATCH_MESSAGE

            /** recv_msgから情報取得 **/
            get_msg_batch_epoch_id = get_epoch_id_from_batch_meg(recv_msg);
            // recv_logに書き込み
            if (config->system_finish_request == 0) {
                pthread_mutex_lock(&config->log_fp_mutex);
                fprintf(config->log_fp, "%d,%lf,%d,%d,%d,%d\n", 1, recv_time,
                        recv_msg_len, BATCH_MESSAGE, 0, get_msg_batch_epoch_id);
                // fflush(config->recv_log_fp);
                pthread_mutex_unlock(&config->log_fp_mutex);

                // pthread_mutex_lock(&config->recv_log_fp_mutex);
                // fprintf(config->recv_log_fp, "%lf,%d,%d\n", recv_time,
                //         BATCH_MESSAGE, recv_msg_len);
                // fflush(config->recv_log_fp);
                // pthread_mutex_unlock(&config->recv_log_fp_mutex);
            }
            // printf("RECV BATCH_MESSAGE epoch_id:%d,len:%d\n",
            //        get_msg_batch_epoch_id, recv_msg_len);
            /**************************/

            if (get_msg_batch_epoch_id == next_batch_epoch_id) {
                next_batch_epoch_id++;

                /** log追加 **/
                append_log(config->batch_log, recv_msg, recv_msg_len,
                           get_msg_batch_epoch_id);
                /*************/

                batch = NULL;
                batch = batch_create(get_msg_batch_epoch_id);
                get_info_from_batch_msg(recv_msg, batch);
                add_node_batch_list(config->write_tx_buffer, batch);

                /** masterにACK送信 **/
                // ACKメッセージ作成
                send_msg_len = create_ack_batch_msg(
                    my_replica_id, get_msg_batch_epoch_id, send_msg);

                // masterにACK送信
                send_port = get_port_master_sequencer_from_slave_sequencer(
                    my_replica_id);
                get_sequencer_ipaddr(0, send_ipaddr);
                udp_cl_send_msg(&cl_info, send_msg, send_msg_len, send_ipaddr,
                                send_port, errmsg);
                send_time = get_time();
                /*********************/

                // send_logに書き込み
                if (config->system_finish_request == 0) {
                    pthread_mutex_lock(&config->log_fp_mutex);
                    fprintf(config->log_fp, "%d,%lf,%d,%d,%d,%d\n", 0,
                            send_time, send_msg_len, ACK_BATCH_MESSAGE, 0,
                            get_msg_batch_epoch_id);
                    // fflush(config->send_log_fp);
                    pthread_mutex_unlock(&config->log_fp_mutex);

                    // pthread_mutex_lock(&config->send_log_fp_mutex);
                    // fprintf(config->send_log_fp, "%lf,%d,%d,0\n", send_time,
                    //         ACK_BATCH_MESSAGE, send_msg_len);
                    // fflush(config->send_log_fp);
                    // pthread_mutex_unlock(&config->send_log_fp_mutex);
                }
                // printf("SEND first ACK_MESSAGE to only master epoch_id:%d\n",
                //        get_msg_batch_epoch_id);

            } else if (get_msg_batch_epoch_id < next_batch_epoch_id) {
                /** masterにACK送信 **/
                // ACKメッセージ作成
                send_msg_len = create_ack_batch_msg(
                    my_replica_id, get_msg_batch_epoch_id, send_msg);

                // masterにACK送信
                send_port = get_port_master_sequencer_from_slave_sequencer(
                    my_replica_id);
                get_sequencer_ipaddr(0, send_ipaddr);
                udp_cl_send_msg(&cl_info, send_msg, send_msg_len, send_ipaddr,
                                send_port, errmsg);
                send_time = get_time();

                // send_logに書き込み
                if (config->system_finish_request == 0) {
                    pthread_mutex_lock(&config->log_fp_mutex);
                    fprintf(config->log_fp, "%d,%lf,%d,%d,%d,%d\n", 0,
                            send_time, send_msg_len, ACK_BATCH_MESSAGE, 0,
                            get_msg_batch_epoch_id);
                    // fflush(config->send_log_fp);
                    pthread_mutex_unlock(&config->log_fp_mutex);

                    // pthread_mutex_lock(&config->send_log_fp_mutex);
                    // fprintf(config->send_log_fp, "%lf,%d,%d,0\n", send_time,
                    //         ACK_BATCH_MESSAGE, send_msg_len);
                    // fflush(config->send_log_fp);
                    // pthread_mutex_unlock(&config->send_log_fp_mutex);
                }
                // printf("SEND ACK_MESSAGE to only master epoch_id:%d\n",
                //        get_msg_batch_epoch_id);
                /*********************/

            } else if (get_msg_batch_epoch_id > next_batch_epoch_id) {
                // 来ないはず
                assert(0);
            }
        }
    }

    udp_cl_socket_deinit(&cl_info);
    udp_sv_socket_deinit(&sv_info);
}