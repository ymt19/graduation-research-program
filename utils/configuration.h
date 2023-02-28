#pragma once

#include <pthread.h>
#include <stdio.h>

#include "../batch/batch_list.h"
#include "../utils/log.h"
#include "../utils/storage.h"
#include "../network/tcp/tcp_client.h"
#include "../network/tcp/tcp_server.h"
#include "../tx/transaction_list.h"

// #define SYSTEM_DURATION 500  // システム稼働期間
#define EPOCH_DURATION 0.1  // エポック期間(s)
#define REPLICA_NUM 7       // レプリカ数（マスタ含む）
#define STORAGE_SIZE 124
#define BATCH_LOG_FILENAME "batch_log"
#define CLIENT_NUM 10  // 1ノードに属するクライアント数
#define MAX_NUM_OF_NODE 10

#define MY_IPADDR "127.0.0.1"
/************    分散環境での実験用     ***********/
// マスタ―スレーブ間の通信
#define MASTER_SEQUENCER_IPADDR "192.168.1.1"
#define SLAVE1_SEQUENCER_IPADDR "192.168.1.2"
#define SLAVE2_SEQUENCER_IPADDR "192.168.1.3"
#define SLAVE3_SEQUENCER_IPADDR "192.168.1.4"
#define SLAVE4_SEQUENCER_IPADDR "192.168.1.5"
#define SLAVE5_SEQUENCER_IPADDR "192.168.1.6"
#define SLAVE6_SEQUENCER_IPADDR "192.168.1.7"

// クライアント-スレーブ間の通信
#define CLIENT_IPADDR "192.168.6.1"
#define SLAVE1_CLIENT_INTERFACE_IPADDR "192.168.6.2"
#define SLAVE2_CLIENT_INTERFACE_IPADDR "192.168.6.3"
#define SLAVE3_CLIENT_INTERFACE_IPADDR "192.168.6.4"
#define SLAVE4_CLIENT_INTERFACE_IPADDR "192.168.6.5"
#define SLAVE5_CLIENT_INTERFACE_IPADDR "192.168.6.6"
#define SLAVE6_CLIENT_INTERFACE_IPADDR "192.168.6.7"
#define MASTER_CLIENT_INTERFACE_IPADDR "192.168.6.8"

// スレーブ―スレーブ間の通信（リージョン内通信）
#define WITHIN_REGION_SLAVE1_IPADDR "192.168.9.1"
#define WITHIN_REGION_SLAVE2_IPADDR "192.168.9.2"
#define WITHIN_REGION_SLAVE3_IPADDR "192.168.9.3"
#define WITHIN_REGION_SLAVE4_IPADDR "192.168.9.4"
#define WITHIN_REGION_SLAVE5_IPADDR "192.168.9.5"
#define WITHIN_REGION_SLAVE6_IPADDR "192.168.9.6"

// // TCPを用いて要求を投げるスレーブ　ー＞　マスタ間のマスタ側ポート番号
// // UDPを用いてバッチを送信するマスタ　ー＞　スレーブのスレーブ側ポート番号
// //
// UDPを用いてACK・REQ・BATCHを送信するスレーブ　ー＞　同一リージョンのスレーブの
// // 同一リージョンのスレーブ側ポート番号
// #define SEQUENCER_PORT 5555
// // クライアント　ー＞　スレーブ間のスレーブ側ポート番号
// #define CLIENT_INTERFACE_PORT 6666
// // UDPを用いてACKを送信するスレーブ　ー＞　マスタ間のマスタ側ポート番号
// #define MASTER_SEQUENCER_FROM_SLAVE1_PORT 7771
// #define MASTER_SEQUENCER_FROM_SLAVE2_PORT 7772
// #define MASTER_SEQUENCER_FROM_SLAVE3_PORT 7773
// #define MASTER_SEQUENCER_FROM_SLAVE4_PORT 7774
// #define MASTER_SEQUENCER_FROM_SLAVE5_PORT 7775
// #define MASTER_SEQUENCER_FROM_SLAVE6_PORT 7776

/**
 * 各スレーブノードのクライアントインターフェースからトランザクションを受信する
 * マスタノードのシーケンサのポート番号（UDP）
 */
// #define PORT_MASTER_SEQUENCER_FROM_CLIENT_INTERFACE 5501
#define PORT_MASTER_SEQUENCER_FROM_SLAVE1_CLIENT_INTERFACE 5100
#define PORT_MASTER_SEQUENCER_FROM_SLAVE2_CLIENT_INTERFACE 5200
#define PORT_MASTER_SEQUENCER_FROM_SLAVE3_CLIENT_INTERFACE 5300
#define PORT_MASTER_SEQUENCER_FROM_SLAVE4_CLIENT_INTERFACE 5400
#define PORT_MASTER_SEQUENCER_FROM_SLAVE5_CLIENT_INTERFACE 5500
#define PORT_MASTER_SEQUENCER_FROM_SLAVE6_CLIENT_INTERFACE 5600
/**
 * 各スレーブノードのシーケンサからACKを受信する
 * マスタノードのシーケンサのポート番号（UDP）
 */
#define PORT_MASTER_SEQUENCER_FROM_SLAVE1_SEQUENCER 6661
#define PORT_MASTER_SEQUENCER_FROM_SLAVE2_SEQUENCER 6662
#define PORT_MASTER_SEQUENCER_FROM_SLAVE3_SEQUENCER 6663
#define PORT_MASTER_SEQUENCER_FROM_SLAVE4_SEQUENCER 6664
#define PORT_MASTER_SEQUENCER_FROM_SLAVE5_SEQUENCER 6665
#define PORT_MASTER_SEQUENCER_FROM_SLAVE6_SEQUENCER 6666

/**
 * 従来手法ではマスタノードのシーケンサからBATCHを受信し、
 * 提案手法ではマスタノードのシーケンサと同一リージョンに属するスレーブノードのシーケンサから
 * BATCH/ACK/REQUESTを受信する
 * スレーブノードのシーケンサのポート番号（UDP）
 */
#define PORT_SLAVE1_SEQUENCER 7771
#define PORT_SLAVE2_SEQUENCER 7772
#define PORT_SLAVE3_SEQUENCER 7773
#define PORT_SLAVE4_SEQUENCER 7774
#define PORT_SLAVE5_SEQUENCER 7775
#define PORT_SLAVE6_SEQUENCER 7776

/**
 * クライアントからトランザクションを受信する
 * スレーブノードのクライアントインターフェースのポート番号（UDP）
 */
#define PORT_SLAVE1_CLINET_INTERFACE_FROM_CLIENT 8881
#define PORT_SLAVE2_CLINET_INTERFACE_FROM_CLIENT 8882
#define PORT_SLAVE3_CLINET_INTERFACE_FROM_CLIENT 8883
#define PORT_SLAVE4_CLINET_INTERFACE_FROM_CLIENT 8884
#define PORT_SLAVE5_CLINET_INTERFACE_FROM_CLIENT 8885
#define PORT_SLAVE6_CLINET_INTERFACE_FROM_CLIENT 8886

/**
 * クライアントから終了要求受信
 */
#define PORT_FINISH_MASTER_FROM_CLIENT 9000
#define PORT_FINISH_SLAVE1_FROM_CLIENT 9001
#define PORT_FINISH_SLAVE2_FROM_CLIENT 9002
#define PORT_FINISH_SLAVE3_FROM_CLIENT 9003
#define PORT_FINISH_SLAVE4_FROM_CLIENT 9004
#define PORT_FINISH_SLAVE5_FROM_CLIENT 9005
#define PORT_FINISH_SLAVE6_FROM_CLIENT 9006
/************(ここまで)分散環境での実験用 ***********/

// マスタノードがスレーブノードへ再送するタイムアウト時間（秒）
#define RTO_SLAVE1 1
#define RTO_SLAVE2 1
#define RTO_SLAVE3 1
#define RTO_SLAVE4 1
#define RTO_SLAVE5 1
#define RTO_SLAVE6 1

// ノードが属するリージョンID
#define REGION_SLAVE1 0
#define REGION_SLAVE2 0
#define REGION_SLAVE3 0
#define REGION_SLAVE4 0
#define REGION_SLAVE5 0
#define REGION_SLAVE6 0

/**
 * @brief 実験タイプ
 *
 */
enum exp_t {
    proposed_method,      // 提案
    conventional_method,  // 従来
    tcp_method,
};
typedef enum exp_t exp_t;

/**
 * @brief 接続している各クライアント情報
 *
 */
struct db_client_info_t {
    // 実行待ちトランザクション
    Tx *wait_for_exe;
};
typedef struct db_client_info_t db_client_info_t;

/**
 * 設定情報
 */
struct Configuration {
    // 実験タイプ
    exp_t exp;

    // // システム稼働期間
    // int system_duration;

    // // システム起動時間
    // int system_startup_time;

    // システム終了要請
    // 1なら終了、0なら継続
    int system_finish_request;

    // エポック期間
    double epoch_duration;

    // 自レプリカID
    int my_replica_id;

    // ストレージ
    Storage *storage;

    // 確定済みバッチのログ
    log_info_t *batch_log;

    // （実行可能）write tx バッファ
    batch_list_t *write_tx_buffer;

    // （実行可能）read tx バッファ
    tx_list_t *read_tx_buffer;

    // クライアント情報
    db_client_info_t db_client_info_set[CLIENT_NUM];

    // ログ情報
    FILE *log_fp;
    char log_filename[20];
    pthread_mutex_t log_fp_mutex;

    // FILE *send_log_fp;
    // char send_log_filename[20];
    // pthread_mutex_t send_log_fp_mutex;

    // FILE *recv_log_fp;
    // char recv_log_filename[20];
    // pthread_mutex_t recv_log_fp_mutex;
};
typedef struct Configuration Config;

Config *create_config(exp_t exp, double epoch_duration, int my_replica_id);
// void set_system_start_time(Config *config);
void set_system_finish_request(Config *config);
// int check_system_finish_time(Config *config, double now);
void free_config(Config *config);
// int get_port_master_sequencer_from_client_interface();
int get_port_master_sequencer_from_client_interface(int replica_id,
                                                    int client_id);
int get_port_master_sequencer_from_slave_sequencer(int replica_id);
int get_port_slave_sequencer(int replica_id);
int get_port_slave_client_interface_from_client(int replica_id);
int get_port_finish_from_client(int replica_id);
void get_sequencer_ipaddr(int replica_id, char *ip_addr);
void get_client_interface_ipaddr(int replica_id, char *ip_addr);
void get_within_region_slave_ipaddr(int replica_id, char *ip_addr);
double get_rto(int replica_id);
int get_region(int replica_id);
