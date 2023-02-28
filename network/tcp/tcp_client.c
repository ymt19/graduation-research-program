/**
 * TCPクライアント側の手順
 * 1. tcp_cl_socket_init()
 * 2. tcp_cl_connect()
 * 3. メッセージ送受信
 *   (a) tcp_cl_receive_msg()
 *   (b) tcp_cl_send_msg()
 * 4. tcp_cl_socket_deinit()
 */

#include "../../network/tcp/tcp_client.h"
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/**
 * @brief      サーバからメッセージを受信する
 * @param[in]  info   クライアント情報
 * @param[out] errmsg エラーメッセージ格納先
 * @return     受信したメッセージ長、失敗ならば-1
 */
int tcp_cl_receive_msg(cl_info_t *info, char *recvmsg, int len, char *errmsg) {
    int recv_msglen = 0;

    // メッセージを受信する
    recv_msglen = recv(info->sd, recvmsg, len, 0);
    if(recv_msglen < 0){
        sprintf(errmsg, "(line:%d) %s", __LINE__, strerror(errno));
        return -1;
    }

    // 受信メッセージを出力する
    // recvmsg[recv_msglen] = '\0';
    // fprintf(stdout, "Received: %s\n", buff);

    return recv_msglen;
}

/**
 * @brief       サーバにメッセージを送信する
 * @param[in]   info   クライアント情報
 * @param[in]   sendmsg メッセージ
 * @param[in]   len メッセージ長
 * @param[out]  errmsg エラーメッセージ格納先
 * @return      成功ならば0、失敗ならば-1を返す
 */
int tcp_cl_send_msg(cl_info_t *info, char* sendmsg, int len, char *errmsg) {
    int rc = 0;

    // メッセージ送信
    rc = send(info->sd, sendmsg, len, 0);
    if(rc != len){
        sprintf(errmsg, "tcp_client.c: (line:%d) %s", __LINE__, strerror(errno));
        return -1;
    }
    // // 送信メッセージ出力
    // fprintf(stdout, "Send: %s\n", sendmsg);

    return 0;
}

/**
 * @brief      サーバに接続する
 * @param[in]  info   クライアント情報
 * @param[out] errmsg エラーメッセージ格納先
 * @return     成功ならば0、失敗ならば-1を返す。
 */
int tcp_cl_connect(cl_info_t *info, char *errmsg) {
    int rc = 0;

    rc = connect(info->sd, (struct sockaddr *)&(info->sv_addr),
                 sizeof(info->sv_addr));
    if(rc != 0){
        sprintf(errmsg, "tcp_client.c: (line:%d) %s", __LINE__, strerror(errno));
        return -1;
    }

    return 0;
}

/**
 * @brief       ソケットの初期化(TCP指定)
 * @param[in]   info   クライアント情報
 * @param[in]   sv_ipaddr 接続先サーバIPアドレス
 * @param[in]   sv_port 接続先サーバポート番号
 * @param[out]  errmsg エラーメッセージ格納先
 * @return      成功ならば0、失敗ならば-1を返す。
 */
int tcp_cl_socket_init(cl_info_t *info, char *sv_ipaddr, unsigned short sv_port, char *errmsg) {
    // ソケットを生成する(TCPを指定)
    info->sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(info->sd < 0){
        sprintf(errmsg, "tcp_client.c: (line:%d) %s", __LINE__, strerror(errno));
        return -1;
    }

    // サーバのアドレス構造体を作成する
    info->sv_addr.sin_family = AF_INET;
    info->sv_addr.sin_addr.s_addr = inet_addr(sv_ipaddr);
    info->sv_addr.sin_port = htons(sv_port);

    return 0;
}

/*!
 * @brief      ソケットの終期化
 * @param[in]  info   クライアント接続情報
 * @return     成功ならば0、失敗ならば-1を返す。
 */
void tcp_cl_socket_deinit(cl_info_t *info) {
    // ソケット破棄
    if(info->sd != 0) close(info->sd);

    return;
}