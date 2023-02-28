#include "../batch/batch.h"
#include "../utils/bytes.h"
#include "../utils/message.h"
#include "../tx/transaction.h"
#include <stdio.h>

/**
 * @brief       メッセージが指定タイプか判定
 * @param       msg
 * @param       type
 * @return      指定タイプなら1、そうでないなら0
 */
int check_msg(char *msg, msg_t expected_type) {
    char type = get_char_from_bytes(msg, MSG_TYPE_OFFSET);
    if (expected_type == type) {
        return 1;
    } else {
        return 0;
    }
}



/**
 * @brief       REQUEST_TERMINATE_MESSAGEからTxを取得
 * @param       msg REQUEST_TERMINATE_MESSAGE
 * @return      terminate flag（1なら終了要求、そうでないなら0）
 */
char get_info_from_request_terminate_msg(char *msg) {
    char terminate_flag;
    terminate_flag = get_char_from_bytes(msg, REQUEST_TERMINATE_FLAG_OFFSET);
    return terminate_flag;
}

/**
 * @brief       REQUEST_TERMINATE_MESSAGE作成
 * @param       terminate_flag 終了要求なら1、そうでないなら0
 * @param       msg
 * @return      メッセージ長
 */
int create_request_terminate_msg(char terminate_flag, char *msg) {
    set_char_to_bytes(msg, MSG_TYPE_OFFSET, REQUEST_TERMINATE_MESSAGE);

    set_char_to_bytes(msg, REQUEST_TERMINATE_FLAG_OFFSET, terminate_flag);

    return REQUEST_TERMINATE_MESSAGE_SIZE;
}



/**
 * @brief       RESPONSE_TERMINATE_MESSAGE作成
 * @param       msg
 * @return      メッセージ長
 */
int create_response_terminate_msg(char *msg) {
    set_char_to_bytes(msg, MSG_TYPE_OFFSET, RESPONSE_TERMINATE_MESSAGE);

    return RESPONSE_TERMINATE_MESSAGE_SIZE;
}




/**
 * @brief       REQUEST_TX_MESSAGEからTxを取得
 * @param       msg REQUEST_TX_MESSAGE
 * @param       tx 取得するTxの格納先
 */
void get_info_from_request_tx_msg(char *msg, Tx *tx) {
    tx->replica_id = get_short_int_from_bytes(msg, REQUEST_TX_REPLICA_ID_OFFSET);
    tx->client_id = get_short_int_from_bytes(msg, REQUEST_TX_CLIENT_ID_OFFSET);
    tx->id = get_short_int_from_bytes(msg, REQUEST_TX_ID_OFFSET);
    tx->type = get_char_from_bytes(msg, REQUEST_TX_TYPE_OFFSET);
    tx->key = get_short_int_from_bytes(msg, REQUEST_TX_KEY_OFFSET);
    tx->value = get_short_int_from_bytes(msg, REQUEST_TX_VALUE_OFFSET);
}

/**
 * @brief       メッセージREQUEST_TX_MESSAGEを作成
 * @param       tx メッセージに記載するトランザクション
 * @param       msg 作成するメッセージの保存先
 */
int create_request_tx_msg(Tx *tx, char *msg) {
    set_char_to_bytes(msg, MSG_TYPE_OFFSET, REQUEST_TX_MESSAGE);

    set_short_int_to_bytes(msg, REQUEST_TX_REPLICA_ID_OFFSET, tx->replica_id);
    set_short_int_to_bytes(msg, REQUEST_TX_CLIENT_ID_OFFSET, tx->client_id);
    set_short_int_to_bytes(msg, REQUEST_TX_ID_OFFSET, tx->id);
    set_short_int_to_bytes(msg, REQUEST_TX_TYPE_OFFSET, tx->type);
    set_short_int_to_bytes(msg, REQUEST_TX_KEY_OFFSET, tx->key);
    set_short_int_to_bytes(msg, REQUEST_TX_VALUE_OFFSET, tx->value);

    return REQUEST_TX_MESSAGE_SIZE;
}




/**
 * @brief       RESPONSE_TX_MESSAGEからTxを取得
 * @param       msg RESPONSE_TX_MESSAGE
 * @param       tx 取得するTxの格納先
 */
void get_info_from_response_tx_msg(char *msg, Tx *tx) {
    tx->replica_id = get_short_int_from_bytes(msg, RESPONSE_TX_REPLICA_ID_OFFSET);
    tx->client_id = get_short_int_from_bytes(msg, RESPONSE_TX_CLIENT_ID_OFFSET);
    tx->id = get_short_int_from_bytes(msg, RESPONSE_TX_ID_OFFSET);
    tx->type = get_char_from_bytes(msg, RESPONSE_TX_TYPE_OFFSET);
    tx->key = get_short_int_from_bytes(msg, RESPONSE_TX_KEY_OFFSET);
    tx->value = get_short_int_from_bytes(msg, RESPONSE_TX_VALUE_OFFSET);
    tx->assigned_epoch_id = get_short_int_from_bytes(msg, RESPONSE_TX_ASSIGNED_EPOCH_ID_OFFSET);
}

/**
 * @brief       メッセージRESPONSE_MESSAGEを作成
 * @param       tx メッセージに記載する実行済みトランザクション
 * @param       msg 作成するメッセージの保存先
 * @note
 * トランザクション内の操作数は1を前提
 */
int create_response_tx_msg(Tx *tx, char *msg) {
    set_char_to_bytes(msg, MSG_TYPE_OFFSET, RESPONSE_TX_MESSAGE);

    set_short_int_to_bytes(msg, RESPONSE_TX_REPLICA_ID_OFFSET, tx->replica_id);
    set_short_int_to_bytes(msg, RESPONSE_TX_CLIENT_ID_OFFSET, tx->client_id);
    set_short_int_to_bytes(msg, RESPONSE_TX_ID_OFFSET, tx->id);
    set_char_to_bytes(msg, RESPONSE_TX_TYPE_OFFSET, tx->type);
    set_short_int_to_bytes(msg, RESPONSE_TX_KEY_OFFSET, tx->key);
    set_short_int_to_bytes(msg, RESPONSE_TX_VALUE_OFFSET, tx->value);
    set_short_int_to_bytes(msg, RESPONSE_TX_ASSIGNED_EPOCH_ID_OFFSET, tx->assigned_epoch_id);

    return RESPONSE_TX_MESSAGE_SIZE;
}




/**
 * @brief       
 */
void get_info_from_add_to_batch_msg(char *msg, Tx *tx) {
    tx->replica_id = get_short_int_from_bytes(msg, ADD_TO_BATCH_TX_REPLICA_ID_OFFSET);
    tx->client_id = get_short_int_from_bytes(msg, ADD_TO_BATCH_TX_CLIENT_ID_OFFSET);
    tx->id = get_short_int_from_bytes(msg, ADD_TO_BATCH_TX_ID_OFFSET);
    tx->type = get_char_from_bytes(msg, ADD_TO_BATCH_TX_TYPE_OFFSET);
    tx->key = get_short_int_from_bytes(msg, ADD_TO_BATCH_TX_KEY_OFFSET);
    tx->value = get_short_int_from_bytes(msg, ADD_TO_BATCH_TX_VALUE_OFFSET);
}

/**
 * 
 */
int create_add_to_batch_msg(Tx *tx, char *msg) {
    set_char_to_bytes(msg, MSG_TYPE_OFFSET, ADD_TO_BATCH_MESSAGE);

    set_short_int_to_bytes(msg, ADD_TO_BATCH_TX_REPLICA_ID_OFFSET, tx->replica_id);
    set_short_int_to_bytes(msg, ADD_TO_BATCH_TX_CLIENT_ID_OFFSET, tx->client_id);
    set_short_int_to_bytes(msg, ADD_TO_BATCH_TX_ID_OFFSET, tx->id);
    set_char_to_bytes(msg, ADD_TO_BATCH_TX_TYPE_OFFSET, tx->type);
    set_short_int_to_bytes(msg, ADD_TO_BATCH_TX_KEY_OFFSET, tx->key);
    set_short_int_to_bytes(msg, ADD_TO_BATCH_TX_VALUE_OFFSET, tx->value);

    return ADD_TO_BATCH_MESSAGE_SIZE;
}




/**
 * @brief
 */
void get_info_from_batch_msg(char *msg, Batch *batch) {
    int tx_num;
    batch->epoch_id = get_short_int_from_bytes(msg, BATCH_EPOCH_ID_OFFSET);
    tx_num = get_short_int_from_bytes(msg, BATCH_TX_NUM_OFFSET);

    int offset = BATCH_TX_INFO_OFFSET;
    Tx *tx;
    for (int i = 0; i < tx_num; i++) {
        tx = create_tx();
        tx->replica_id = get_short_int_from_bytes(msg, offset + BATCH_TX_INFO_REPLICA_ID_OFFSET);
        tx->client_id = get_short_int_from_bytes(msg, offset + BATCH_TX_INFO_CLIENT_ID_OFFSET);
        tx->id = get_short_int_from_bytes(msg, offset + BATCH_TX_INFO_TX_ID_OFFSET);
        tx->type = get_char_from_bytes(msg, offset + BATCH_TX_INFO_TX_TYPE_OFFSET);
        tx->key = get_short_int_from_bytes(msg, offset + BATCH_TX_INFO_TX_KEY_OFFSET);
        tx->value = get_short_int_from_bytes(msg, offset + BATCH_TX_INFO_TX_VALUE_OFFSET);
        batch_add_tx(batch, tx);
        offset += BATCH_TX_INFO_MESSAGE_SIZE;
    }
}

/**
 * @brief Get the epoch id from batch meg object
 * 
 * @param msg 
 * @return int 
 */
int get_epoch_id_from_batch_meg(char *msg) {
    int epoch_id;
    return get_short_int_from_bytes(msg, BATCH_EPOCH_ID_OFFSET);
}

/**
 * 
 */
int create_batch_msg(Batch *batch, char *msg, int len) {
    int offset;

    set_char_to_bytes(msg, MSG_TYPE_OFFSET, BATCH_MESSAGE);

    set_short_int_to_bytes(msg, BATCH_EPOCH_ID_OFFSET, batch->epoch_id);
    set_short_int_to_bytes(msg, BATCH_TX_NUM_OFFSET, batch->tx_num);
    
    offset = BATCH_TX_INFO_OFFSET;
    for (int i = 0; i < batch->tx_num; i++) {
        Tx *tx = batch->tx_set[i];
        set_short_int_to_bytes(msg, offset + BATCH_TX_INFO_REPLICA_ID_OFFSET, tx->replica_id);
        set_short_int_to_bytes(msg, offset + BATCH_TX_INFO_CLIENT_ID_OFFSET, tx->client_id);
        set_short_int_to_bytes(msg, offset + BATCH_TX_INFO_TX_ID_OFFSET, tx->id);
        set_char_to_bytes(msg, offset + BATCH_TX_INFO_TX_TYPE_OFFSET, tx->type);
        set_short_int_to_bytes(msg, offset + BATCH_TX_INFO_TX_KEY_OFFSET, tx->key);
        set_short_int_to_bytes(msg, offset + BATCH_TX_INFO_TX_VALUE_OFFSET, tx->value);
        
        offset += BATCH_TX_INFO_MESSAGE_SIZE;
    }

    return offset;
}

/**
 * @brief Get the info from ack msg object
 * 
 * @param msg 
 * @param your_replica_id    送信元レプリカID
 * @param epoch_id 送信元レプリカIDが受信したバッチエポックID
 */
void get_info_from_ack_batch_msg(char *msg, int *your_replica_id, int *epoch_id) {
    *your_replica_id = (int)get_short_int_from_bytes(msg, ACK_BATCH_MY_REPLICA_ID_OFFSET);
    *epoch_id = (int)get_short_int_from_bytes(msg, ACK_BATCH_EPOCH_ID_OFFSET);
}

/**
 * @brief Create a ack msg object
 * 
 * @param my_replica_id 送信元レプリカID
 * @param epoch_id 送信元レプリカが受信したバッチエポックID
 * @param msg 
 * @return int 
 */
int create_ack_batch_msg(int my_replica_id, int epoch_id, char *msg) {
    set_char_to_bytes(msg, MSG_TYPE_OFFSET, ACK_BATCH_MESSAGE);
    set_short_int_to_bytes(msg, ACK_BATCH_MY_REPLICA_ID_OFFSET, (short)my_replica_id);
    set_short_int_to_bytes(msg, ACK_BATCH_EPOCH_ID_OFFSET, (short)epoch_id);
    return ACK_BATCH_MESSAGE_SIZE;
}

/**
 * @brief Get the info from request batch message object
 * 
 * @param msg 
 * @param your_replica_id 送信元レプリカID
 * @param epoch_id 送信元レプリカが送信要求しているバッチエポックID
 */
void get_info_from_request_batch_message(char *msg, int *your_replica_id, int *epoch_id) {
    *your_replica_id = (int)get_short_int_from_bytes(msg, REQUEST_BATCH_MY_REPLICA_ID_OFFSET);
    *epoch_id = (int)get_short_int_from_bytes(msg, REQUEST_BATCH_EPOCH_ID_OFFSET);
}

/**
 * @brief Create a request batch msg object
 * 
 * @param my_replica_id 送信元レプリカID
 * @param epoch_id 送信元レプリカが送信要求するバッチエポックID
 * @param msg 
 * @return int 
 */
int create_request_batch_msg(int my_replica_id, int epoch_id, char *msg) {
    set_char_to_bytes(msg, MSG_TYPE_OFFSET, REQUEST_BATCH_MESSAGE);
    set_short_int_to_bytes(msg, REQUEST_BATCH_MY_REPLICA_ID_OFFSET, (short)my_replica_id);
    set_short_int_to_bytes(msg, REQUEST_BATCH_EPOCH_ID_OFFSET, (short)epoch_id);
    return REQUEST_BATCH_MESSAGE_SIZE;
}

/**
 * @brief Get the info from ack add msg object
 * 
 * @param msg 
 * @param replica_id 
 * @param client_id 
 * @param tx_id 
 */
void get_info_from_ack_add_msg(char *msg, int *replica_id, int *client_id, int *tx_id) {
    *replica_id = (int)get_short_int_from_bytes(msg, ACK_ADD_TX_REPLICA_ID_OFFSET);
    *client_id = (int)get_short_int_from_bytes(msg, ACK_ADD_TX_CLIENT_ID_OFFSET);
    *tx_id = (int)get_short_int_from_bytes(msg, ACK_ADD_TX_ID_OFFSET);
}

/**
 * @brief Create a ack add msg object
 * 
 * @param replica_id 
 * @param client_id 
 * @param tx_id 
 * @param msg 
 * @return int 
 */
int create_ack_add_msg(int replica_id, int client_id, int tx_id, char *msg) {
    set_char_to_bytes(msg, MSG_TYPE_OFFSET, ACK_ADD_MESSAGE);
    set_short_int_to_bytes(msg, ACK_ADD_TX_REPLICA_ID_OFFSET, (short)replica_id);
    set_short_int_to_bytes(msg, ACK_ADD_TX_CLIENT_ID_OFFSET, (short)client_id);
    set_short_int_to_bytes(msg, ACK_ADD_TX_ID_OFFSET, (short)tx_id);
    return ACK_ADD_MESSAGE_SIZE;
}