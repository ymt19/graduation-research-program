#pragma once

enum transaction_type {
    READ_ONLY,
    WRITE_ONLY,
    ERROR,

    FINISH_REQUEST,         // クライアントの接続終了要求
};
typedef enum transaction_type tx_t;

/**
 * トランザクション
 */
struct Transaction {
    // 要求したクライアントが属するレプリカID
    short replica_id;

    // 要求したクライアントのクライアントID
    short client_id;

    // 各クライアントに一意なトランザクションID
    unsigned short id;

    // トランザクションタイプ
    tx_t type;

    // 書き込み先のキーと書き込む値
    short key;
    short value;

    // このトランザクションが割り当てられたエポック
    short assigned_epoch_id;

    // このトランザクションの実行が完了したら1、未だなら0
    char complete_flag;
};
typedef struct Transaction Tx;

Tx *create_tx();
void init_tx(Tx *tx);
void modify_to_write_tx(Tx *tx, short key, short value);
void modify_to_read_tx(Tx *tx, short key);
void free_tx(Tx *tx);
void set_assigned_epoch_id_to_tx(Tx *tx, short assigned_epoch_id);
void set_client_info_to_tx(Tx *tx, short client_replica_id, short client_id, short id);
void raise_tx_complete_flag(Tx *tx);
void tx_cpy(Tx *dst, Tx src);
void print_tx_info(Tx *tx, char *msg);
void modify_finish_request(Tx *tx);