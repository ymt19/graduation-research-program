#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../utils/configuration.h"
#include "../master/master_sequencer.h"
#include "../utils/message.h"
#include "../scheduler/read_scheduler.h"
#include "../network/tcp/tcp_client.h"
#include "../network/udp/udp_server.h"
#include "../scheduler/write_scheduler.h"

void request_terminate_client_interface();

int main(int argc, char *argv[]) {
    int ret;
    int my_replica_id = 0;
    exp_t exp;
    pthread_t sequencer_thread, read_scheduler_thread, write_scheduler_thread;
    Config *config;

    if (argc < 2) {
        fprintf(stdout, "<prop|conv|tcp>\n");
        exit(1);
    }
    if (strcmp("prop", argv[1]) == 0) {
        exp = proposed_method;
    } else if (strcmp("conv", argv[1]) == 0) {
        exp = conventional_method;
    } else if (strcmp("tcp", argv[1]) == 0) {
        exp = tcp_method;
    } else {
        fprintf(stdout, "<prop|conv|tcp>\n");
        exit(1);
    }

    // configを定義
    config = create_config(exp, EPOCH_DURATION, my_replica_id);
    config->system_finish_request = -1;

    // sequencerの起動
    ret = pthread_create(&sequencer_thread, NULL,
                         (void *)master_sequencer_startup, (void *)config);
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
    // write_scheduler_startup(config);
    ret = pthread_create(&write_scheduler_thread, NULL,
                         (void *)write_scheduler_startup, (void *)config);
    if (ret != 0) {
        // error処理
    }

    udp_sv_info_t udp_sv_info;
    int finish_port = get_port_finish_from_client(config->my_replica_id);
    char errmsg[256];
    char recv_msg[256];
    int recv_msg_len;
    udp_sv_socket_init(&udp_sv_info, finish_port, 0, errmsg);
    while (1) {
        udp_sv_receive_msg(&udp_sv_info, recv_msg, 256, errmsg);
        if (strcmp(recv_msg, "finish") == 0) {
            printf("get fisnish\n");
            break;
        } else if (strcmp(recv_msg, "start") == 0) {
            printf("get start\n");
            config->system_finish_request = 0;
        }
    }
    config->system_finish_request = 1;
    udp_sv_socket_deinit(&udp_sv_info);

    ret = pthread_join(sequencer_thread, NULL);
    if (ret != 0) {
        // error処理
    }
    ret = pthread_join(read_scheduler_thread, NULL);
    if (ret != 0) {
        // error処理
    }
    ret = pthread_join(write_scheduler_thread, NULL);
    if (ret != 0) {
        // error処理
    }

    // 最終結果表示
    for (int i = 0; i < STORAGE_SIZE; i++) {
        printf("key[%d]: value[%d]\n", i, config->storage->records[i].value);
    }

    free_config(config);
}