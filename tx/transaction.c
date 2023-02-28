#include "../tx/transaction.h"
#include "../utils/bytes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief       Txのメモリ領域を確保
 * @return      確保したメモリ領域
 */
Tx *create_tx() {
    Tx *tx = malloc(sizeof(Tx));
    init_tx(tx);
    // tx->replica_id = -1;
    // tx->client_id = -1;
    // tx->id = -1;
    // tx->type = ERROR;
    // tx->key = -1;
    // tx->value = -1;
    // tx->assigned_epoch_id = -1;
    // tx->complete_flag = 0;
    return tx;
}

/**
 * @brief       メモリ確保済みTxのメンバを初期化
 * @param       tx 初期化するTx
 */
void init_tx(Tx *tx) {
    tx->replica_id = -1;
    tx->client_id = -1;
    tx->id = -1;
    tx->type = ERROR;
    tx->key = -1;
    tx->value = -1;
    tx->assigned_epoch_id = -1;
    tx->complete_flag = 0;
}

/**
 * @brief       TxをWriteトランザクションに修正
 * @param       tx
 * @param       key 書き込み先キー
 * @param       value 書き込む値
 * @return      確保したメモリ領域
 * @note        complete flagを降ろす
 */
void modify_to_write_tx(Tx *tx, short key, short value) {
    // 指定値を代入
    tx->type = WRITE_ONLY;
    tx->key = key;
    tx->value = value;
    tx->complete_flag = 0;
}

/**
 * @brief       TxをReadトランザクションに修正
 * @param       tx
 * @param       key 読み込み先キー
 * @return      確保したメモリ領域
 * @note        complete flagを降ろす
 */
void modify_to_read_tx(Tx *tx, short key) {
    // 指定地を代入
    tx->type = READ_ONLY;
    tx->key = key;
    tx->value = -1;
    tx->complete_flag = 0;
}

/**
 * @brief       Txのメモリ領域を解放
 * @param       tx 解放するメモリ領域
 */
void free_tx(Tx *tx) {
    free(tx);
}

/**
 * @brief       TxにエポックIDを設定
 * @param       tx
 * @param       epoch_id
 */
void set_assigned_epoch_id_to_tx(Tx *tx, short assigned_epoch_id) {
    tx->assigned_epoch_id = assigned_epoch_id;
}

/**
 * @brief       Txに要求元クライアント情報を設定
 * @param       tx 情報を加えるトランザクション
 * @param       client_replica_id
 * @param       client_id
 * @param       id トランザクションID
 */
void set_client_info_to_tx(Tx *tx, short client_replica_id, short client_id, short tx_id) {
    tx->replica_id = client_replica_id;
    tx->client_id = client_id;
    tx->id = tx_id;
}

/**
 * @brief       Txのcomplete flagを立てる
 * @param       tx
 */
void raise_tx_complete_flag(Tx *tx) {
    tx->complete_flag = 1;
}

void tx_cpy(Tx *dst, Tx src) {
    memset(dst, 0, sizeof(Tx));
    dst->replica_id = src.replica_id;
    dst->client_id = src.client_id;
    dst->id = src.id;
    dst->type = src.type;
    dst->key = src.key;
    dst->value = src.value;
    dst->assigned_epoch_id = src.assigned_epoch_id;
    dst->complete_flag = src.complete_flag;
}

void print_tx_info(Tx *tx, char *msg) {
    printf("%s rid%d,cid%d,tid%d,ttype%d,key%d,value%d,epoch%d,\n", msg,
    tx->replica_id, tx->client_id, tx->id, tx->type, tx->key, tx->value,tx->assigned_epoch_id);
}

void modify_finish_request(Tx *tx) {
    init_tx(tx);
    tx->type = FINISH_REQUEST;
}