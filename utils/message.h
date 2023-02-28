#pragma once

#include "../batch/batch.h"

// メッセージのヘッダ
#define MSG_TYPE_OFFSET 0

// REQUEST_TERMINATE_MESSAGE
#define REQUEST_TERMINATE_FLAG_OFFSET MSG_TYPE_OFFSET + sizeof(char)
#define REQUEST_TERMINATE_MESSAGE_SIZE \
    REQUEST_TERMINATE_FLAG_OFFSET + sizeof(char)

// RESPONSE_TERMINATE_MESSAGE
#define RESPONSE_TERMINATE_MESSAGE_SIZE MSG_TYPE_OFFSET + sizeof(char)

// REQUEST_TX_MESSAGEのオフセット
#define REQUEST_TX_REPLICA_ID_OFFSET MSG_TYPE_OFFSET + sizeof(char)
#define REQUEST_TX_CLIENT_ID_OFFSET REQUEST_TX_REPLICA_ID_OFFSET + sizeof(short)
#define REQUEST_TX_ID_OFFSET REQUEST_TX_CLIENT_ID_OFFSET + sizeof(short)
#define REQUEST_TX_TYPE_OFFSET REQUEST_TX_ID_OFFSET + sizeof(short)
#define REQUEST_TX_KEY_OFFSET REQUEST_TX_TYPE_OFFSET + sizeof(char)
#define REQUEST_TX_VALUE_OFFSET REQUEST_TX_KEY_OFFSET + sizeof(short)
#define REQUEST_TX_MESSAGE_SIZE REQUEST_TX_VALUE_OFFSET + sizeof(short)

// RESPONSE_TX_MESSAGEのオフセット
#define RESPONSE_TX_REPLICA_ID_OFFSET MSG_TYPE_OFFSET + sizeof(char)
#define RESPONSE_TX_CLIENT_ID_OFFSET \
    RESPONSE_TX_REPLICA_ID_OFFSET + sizeof(short)
#define RESPONSE_TX_ID_OFFSET RESPONSE_TX_CLIENT_ID_OFFSET + sizeof(short)
#define RESPONSE_TX_TYPE_OFFSET RESPONSE_TX_ID_OFFSET + sizeof(short)
#define RESPONSE_TX_KEY_OFFSET RESPONSE_TX_TYPE_OFFSET + sizeof(char)
#define RESPONSE_TX_VALUE_OFFSET RESPONSE_TX_KEY_OFFSET + sizeof(short)
#define RESPONSE_TX_ASSIGNED_EPOCH_ID_OFFSET \
    RESPONSE_TX_VALUE_OFFSET + sizeof(short)
#define RESPONSE_TX_MESSAGE_SIZE \
    RESPONSE_TX_ASSIGNED_EPOCH_ID_OFFSET + sizeof(short)

// ADD_TO_BATCH_MESSAGEのオフセット
#define ADD_TO_BATCH_TX_REPLICA_ID_OFFSET MSG_TYPE_OFFSET + sizeof(char)
#define ADD_TO_BATCH_TX_CLIENT_ID_OFFSET \
    ADD_TO_BATCH_TX_REPLICA_ID_OFFSET + sizeof(short)
#define ADD_TO_BATCH_TX_ID_OFFSET \
    ADD_TO_BATCH_TX_CLIENT_ID_OFFSET + sizeof(short)
#define ADD_TO_BATCH_TX_TYPE_OFFSET ADD_TO_BATCH_TX_ID_OFFSET + sizeof(short)
#define ADD_TO_BATCH_TX_KEY_OFFSET ADD_TO_BATCH_TX_TYPE_OFFSET + sizeof(char)
#define ADD_TO_BATCH_TX_VALUE_OFFSET ADD_TO_BATCH_TX_KEY_OFFSET + sizeof(short)
#define ADD_TO_BATCH_MESSAGE_SIZE ADD_TO_BATCH_TX_VALUE_OFFSET + sizeof(short)

/**
 * BATCH_MESSAGEのメッセージのオフセット
 * BATCH_TX_NUM_OFFSET以降は1トランザクションに対しREQUEST_TX_MESSAGEを用いて
 * バッチ内のすべてのトランザクション情報を表す。
 */
#define BATCH_EPOCH_ID_OFFSET MSG_TYPE_OFFSET + sizeof(char)
#define BATCH_TX_NUM_OFFSET BATCH_EPOCH_ID_OFFSET + sizeof(short)
#define BATCH_TX_INFO_OFFSET BATCH_TX_NUM_OFFSET + sizeof(short)
/**
 * BATCH_MESSAGEのBATCH_TX_INFO_OFFSET内におけるオフセット
 * トランザクション数分このオフセットが連続している。
 */
#define BATCH_TX_INFO_REPLICA_ID_OFFSET 0
#define BATCH_TX_INFO_CLIENT_ID_OFFSET \
    BATCH_TX_INFO_REPLICA_ID_OFFSET + sizeof(short)
#define BATCH_TX_INFO_TX_ID_OFFSET \
    BATCH_TX_INFO_CLIENT_ID_OFFSET + sizeof(short)
#define BATCH_TX_INFO_TX_TYPE_OFFSET BATCH_TX_INFO_TX_ID_OFFSET + sizeof(short)
#define BATCH_TX_INFO_TX_KEY_OFFSET BATCH_TX_INFO_TX_TYPE_OFFSET + sizeof(char)
#define BATCH_TX_INFO_TX_VALUE_OFFSET \
    BATCH_TX_INFO_TX_KEY_OFFSET + sizeof(short)
#define BATCH_TX_INFO_MESSAGE_SIZE BATCH_TX_INFO_TX_VALUE_OFFSET + sizeof(short)

/**
 * ACK_BATCH_MESSAGEのフォーマット
 */
#define ACK_BATCH_MY_REPLICA_ID_OFFSET MSG_TYPE_OFFSET + sizeof(char)
#define ACK_BATCH_EPOCH_ID_OFFSET ACK_BATCH_MY_REPLICA_ID_OFFSET + sizeof(short)
#define ACK_BATCH_MESSAGE_SIZE ACK_BATCH_EPOCH_ID_OFFSET + sizeof(short)

/**
 * REQUEST_BATCH_MESSAGEのフォーマット
 */
#define REQUEST_BATCH_MY_REPLICA_ID_OFFSET MSG_TYPE_OFFSET + sizeof(char)
#define REQUEST_BATCH_EPOCH_ID_OFFSET \
    REQUEST_BATCH_MY_REPLICA_ID_OFFSET + sizeof(short)
#define REQUEST_BATCH_MESSAGE_SIZE REQUEST_BATCH_EPOCH_ID_OFFSET + sizeof(short)

/**
 * ACK_ADD_MESSAGE
 */
#define ACK_ADD_TX_REPLICA_ID_OFFSET MSG_TYPE_OFFSET + sizeof(char)
#define ACK_ADD_TX_CLIENT_ID_OFFSET ACK_ADD_TX_REPLICA_ID_OFFSET + sizeof(short)
#define ACK_ADD_TX_ID_OFFSET ACK_ADD_TX_CLIENT_ID_OFFSET + sizeof(short)
#define ACK_ADD_MESSAGE_SIZE ACK_ADD_TX_ID_OFFSET + sizeof(short)

enum message_type {
    ERROR_MESSAGE,

    /**
     * client interfaceが接続を確認した直後に送受信するメッセージ。
     * client <--> client interface
     * main   <--> client interface
     */
    REQUEST_TERMINATE_MESSAGE,   // main() or client -> client interface
    RESPONSE_TERMINATE_MESSAGE,  // client interface -> client or main()

    /**
     * clientがclient
     * interfaceに送信するトランザクション要求とその結果を返すメッセージ。
     * client <--> client interface
     */
    REQUEST_TX_MESSAGE,   // client -> client interface
    RESPONSE_TX_MESSAGE,  // client interface -> client

    /**
     * マスタノードが決定したバッチをスレーブノードに送信するメッセージ。
     * この一つのメッセージがバッチの中の一つのトランザクションを表す。
     * client interface(slave) --> master sequencer(master) --> slave
     * sequencer(slave)
     */
    ADD_TO_BATCH_MESSAGE,  // client interface -> master sequencer
    ACK_ADD_MESSAGE,
    BATCH_MESSAGE,  // master sequcner -> slave sequencer
    ACK_BATCH_MESSAGE,
    REQUEST_BATCH_MESSAGE,
};
typedef enum message_type msg_t;

int check_msg(char *msg, msg_t expected_type);

char get_info_from_request_terminate_msg(char *msg);
int create_request_terminate_msg(char terminate_flag, char *msg);

int create_response_terminate_msg(char *msg);

void get_info_from_request_tx_msg(char *msg, Tx *tx);
int create_request_tx_msg(Tx *tx, char *msg);

void get_info_from_response_tx_msg(char *msg, Tx *tx);
int create_response_tx_msg(Tx *tx, char *msg);

void get_info_from_add_to_batch_msg(char *msg, Tx *tx);
int create_add_to_batch_msg(Tx *tx, char *msg);

void get_info_from_batch_msg(char *msg, Batch *batch);
int get_epoch_id_from_batch_meg(char *msg);
int create_batch_msg(Batch *batch, char *msg, int len);

void get_info_from_ack_batch_msg(char *msg, int *your_replica_id,
                                 int *epoch_id);
int create_ack_batch_msg(int my_replica_id, int epoch_id, char *msg);

void get_info_from_request_batch_message(char *msg, int *your_replica_id,
                                         int *epoch_id);
int create_request_batch_msg(int my_replica_id, int epoch_id, char *msg);

void get_info_from_ack_add_msg(char *msg, int *replica_id, int *client_id,
                               int *tx_id);
int create_ack_add_msg(int replica_id, int client_id, int tx_id, char *msg);
