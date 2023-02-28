#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../client_interface/client_interface.h"
#include "../utils/configuration.h"
#include "../utils/message.h"
#include "../scheduler/read_scheduler.h"
#include "../slave/slave_sequencer.h"
#include "../network/tcp/tcp_client.h"
#include "../scheduler/write_scheduler.h"

void request_terminate_client_interface(Config *config);

int main(int argc, char *argv[]) {
    int ret;
    int my_replica_id;
    exp_t exp;
    pthread_t client_interface_thread, sequencer_thread, read_scheduler_thread;
    Config *config;

    if (argc < 3) {
        fprintf(stdout, "<replica id> <prop|conv|tcp>\n");
        exit(1);
    }
    my_replica_id = atoi(argv[1]);
    if (strcmp("prop", argv[2]) == 0) {
        exp = proposed_method;
    } else if (strcmp("conv", argv[2]) == 0) {
        exp = conventional_method;
    } else if (strcmp("tcp", argv[2]) == 0) {
        exp = tcp_method;
    } else {
        fprintf(stdout, "<replica id> <prop|conv|tcp>\n");
        exit(1);
    }

    // configを定義
    config = create_config(exp, EPOCH_DURATION, my_replica_id);

    // client interface起動
    ret = pthread_create(&client_interface_thread, NULL,
                         (void *)client_interface_startup, (void *)config);
    if (ret != 0) {
        // error処理
    }

    // sequencerの起動
    ret = pthread_create(&sequencer_thread, NULL, (void *)slave_sequencer,
                         (void *)config);
    if (ret != 0) {
        // error処理
    }

    // read scheduler起動
    ret = pthread_create(&read_scheduler_thread, NULL,
                         (void *)read_scheduler_startup, (void *)config);
    if (ret != 0) {
        // error処理
    }

    // write scheduler起動
    write_scheduler_startup(config);

    // client interfaceに終了要求
    // request_terminate_client_interface(config);

    ret = pthread_join(client_interface_thread, NULL);
    printf("client interface finish\n");
    if (ret != 0) {
        // error処理
    }
    ret = pthread_join(sequencer_thread, NULL);
    printf("sequencer thread finish\n");
    if (ret != 0) {
        // error処理
    }
    ret = pthread_join(read_scheduler_thread, NULL);
    printf("read scheduler finish\n");
    if (ret != 0) {
        // error処理
    }

    // 最終結果表示
    for (int i = 0; i < STORAGE_SIZE; i++) {
        printf("key[%d]: value[%d]\n", i, config->storage->records[i].value);
    }

    free_config(config);
}

/**
 * @brief       client interfaceに終了要求送信
 */
// void request_terminate_client_interface(Config *config) {
//     cl_info_t cl_info;
//     int len;
//     char msg[BUFSIZ];
//     char errmsg[256];

//     /************* client interfaceの終了 *****************/
//     // 自ノードのclient interfaceに接続
//     // tcp_cl_socket_init(&cl_info, SLAVE_NODE_IPADDR, CLIENT_INTERFACE_PORT,
//     errmsg); tcp_cl_socket_init(&cl_info, MY_IPADDR,
//     get_client_interface_port(config->my_replica_id), errmsg);

//     tcp_cl_connect(&cl_info, errmsg);
//     // client interface終了要求メッセージ送信
//     len = create_request_terminate_msg(1, msg);
//     tcp_cl_send_msg(&cl_info, msg, len, errmsg);
//     // client interface終了要求応答メッセージ受信
//     len = tcp_cl_receive_msg(&cl_info, msg, BUFSIZ, errmsg);
//     tcp_cl_socket_deinit(&cl_info);
// }