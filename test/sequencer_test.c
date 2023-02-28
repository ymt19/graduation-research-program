// Client
#include "tcp_client.h"
#include "transaction.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// バッチ間隔(ms)
#define BATCH_INTERVAL 10000
// 接続先IPアドレス
#define SCHED_IPADDR "127.0.0.1"
// 接続先ポート番号
#define SCHED_PORT 5555


int main(int argc, char *argv[]) {
    int rc = 0;
    cl_info_t info = {0};
    char errmsg[BUFSIZ];

    rc = tcp_cl_socket_init(&info, SCHED_IPADDR, SCHED_PORT,
                            errmsg);
    if(rc != 0){
        fprintf(stderr, "Error: %s\n", errmsg);
        return(-1);
    }
    
    rc = tcp_cl_connect(&info, errmsg);
    if(rc != 0){
        fprintf(stderr, "Error: %s\n", errmsg);
        return(-1);
    }

    // メッセージ送受信
    int cnt = 0;
    int len;
    char msg[BUFSIZ];
    transaction tx1_1, tx1_2, tx2_1, tx2_2;

    // tx1
    tx1_1 = new_transaction(0, WRITE);
    add_write_op(&tx1_1, 0, 100);
    add_write_op(&tx1_1, 10, 10000);
    add_write_op(&tx1_1, 30, 22222);
    len = tx_to_bytes(tx1_1, msg, BUFSIZ);

    rc = tcp_cl_send_msg(&info, msg, len, errmsg);
    if(rc != 0){
        fprintf(stderr, "Error: %s\n", errmsg);
        return -1;
    }
    len = tcp_cl_receive_msg(&info, msg, BUFSIZ, errmsg);
    if(len < 0){
        fprintf(stderr, "Error: %s\n", errmsg);
        return -1;
    }

    tx1_2 = new_transaction(1, WRITE_SET);
    bytes_to_tx(msg, len, &tx1_2);

    // tx2
    tx2_1 = new_transaction(2, READ_SET); 
    add_read_op(&tx2_1, 0);
    add_read_op(&tx2_1, 10);
    add_read_op(&tx2_1, 30);
    len = tx_to_bytes(tx2_1, msg, BUFSIZ);

    rc = tcp_cl_send_msg(&info, msg, len, errmsg);
    if(rc != 0){
        fprintf(stderr, "Error: %s\n", errmsg);
        return -1;
    }
    len = tcp_cl_receive_msg(&info, msg, BUFSIZ, errmsg);
    if(len < 0){
        fprintf(stderr, "Error: %s\n", errmsg);
        return -1;
    }

    tx2_2 = new_transaction(3, READ_SET);
    bytes_to_tx(msg, len, &tx2_2);
    assert(tx2_2.operations[0].value == 100);
    assert(tx2_2.operations[0].key == 0);
    assert(tx2_2.operations[1].value == 10000);
    assert(tx2_2.operations[1].key == 10);
    assert(tx2_2.operations[2].value == 22222);
    assert(tx2_2.operations[2].key == 30);

    // scheduler終了要請
    sprintf(msg, "%s", "terminate");
    rc = tcp_cl_send_msg(&info, msg, strlen(msg), errmsg);
    if(rc != 0){
        fprintf(stderr, "Error: %s\n", errmsg);
        return -1;
    }

    len = tcp_cl_receive_msg(&info, msg, BUFSIZ, errmsg);
    if(len < 0){
        fprintf(stderr, "Error: %s\n", errmsg);
        return -1;
    }
    msg[len] = '\0';
    printf("%s\n", msg);

    tcp_cl_socket_deinit(&info);

    return(0);
}