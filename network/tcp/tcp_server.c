/**
 * TCPサーバ側手順
 * 1. tcp_sv_sock_init()
 * 2. connected_sd = tcp_sv_accept() 接続完了
 * 3. メッセージ送受信
 *      (a) tcp_sv_receive_msg() <- connected_sd
 *      (b) tcp_sv_send_msg() <- connected_sd
 * 4. tcp_sv_deinit()
 */
#include "../../network/tcp/tcp_server.h"
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/**
 * @brief       クライアントからのメッセージを受信する
 * @param[in]   connected_sd 接続済みソケットディスクリプタ
 * @param[out]  buff 受信メッセージ格納先
 * @param[in]   len buffのサイズ
 * @param[out]  errmsg エラーメッセージ格納先
 * @return     受信したメッセージ長、失敗ならば-1を返す。
 */
int tcp_sv_receive_msg(int connected_sd, char *buff, int len, char *errmsg) {
    int recv_msglen = 0;

    // クライアントからメッセージ受信
    recv_msglen = recv(connected_sd, buff, len, 0);
    if (recv_msglen < 0) {
        sprintf(errmsg, "(line:%d) %s", __LINE__, strerror(errno));
        return -1;
    }

    // // 受信メッセージを出力する
    // buff[recv_msglen] = '\0';
    // fprintf(stdout, "Received: %s\n", buff);

    return recv_msglen;
}

/**
 * @brief       クライアントへメッセージを送信する
 * @param[in]   connected_sd 接続済みソケットディスクリプタ
 * @param[in]   buff 送信メッセージ
 * @param[in]   len 送信メッセージ長
 * @param[out]  errmsg エラーメッセージ格納先
 * @return      成功0、失敗-1
 */
int tcp_sv_send_msg(int connected_sd, char *sendmsg, int len, char *errmsg) {
    int rc = 0;

    rc = send(connected_sd, sendmsg, len, 0);
    if(rc != len){
        sprintf(errmsg, "tcp_server.c: (line:%d) %s", __LINE__, strerror(errno));
        return -1;
    }

    return 0;
}

/**
 * @brief       クライアントから受けた接続要求から接続済みソケットを生成
 * @param[in]   info サーバ情報
 * @param[out]  errmsg エラーメッセージ格納先
 * @return      接続済みのソケットディスクリプタ，失敗したら-1
 */
int tcp_sv_accept(sv_info_t *info, char *errmsg) {
    int connected_sd;
    int len = sizeof(info->cl_addr);

    connected_sd = accept(info->listen_sd, (struct sockaddr *)&(info->cl_addr),
                                (socklen_t*)&len);
    if(connected_sd < 0){
        sprintf(errmsg, "tcp_server.c: (line:%d) %s", __LINE__, strerror(errno));
        return -1;
    }

    // 接続したクライアントの情報を出力
    fprintf(stdout, "connect with %s\n", inet_ntoa(info->cl_addr.sin_addr));
    return connected_sd;
}

/**
 * @brief       ソケットの初期化
 * @param[in]   info サーバ情報
 * @param[in]   sv_port サーバポート番号
 * @param[in]   backlog 保留中の接続のキューサイズ
 * @param[out]  errmsg エラーメッセージ格納先
 * @return      成功0、失敗-1
 */
int tcp_sv_socket_init(sv_info_t *info, unsigned short sv_port, int backlog, char *errmsg) {
    int rc = 0;

    // ソケットを生成する(TCPを指定)
    info->listen_sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(info->listen_sd < 0){
        sprintf(errmsg, "tcp_server.c: (line:%d) %s", __LINE__, strerror(errno));
        return -1;
    }

    // サーバのアドレス構造体を作成する
    info->sv_addr.sin_family = AF_INET;
    info->sv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    info->sv_addr.sin_port = htons(sv_port);

    // ローカルアドレスへバインドする
    rc = bind(info->listen_sd, (struct sockaddr *)&(info->sv_addr), sizeof(info->sv_addr));
    if(rc != 0){
        sprintf(errmsg, "tcp_server.c: (line:%d) %s", __LINE__, strerror(errno));
        return -1;
    }

    // ソケットを接続待ち状態にする
    rc = listen(info->listen_sd, backlog);
    if(rc != 0){
        sprintf(errmsg, "tcp_server.c: (line:%d) %s", __LINE__, strerror(errno));
        return -1;
    }

    return 0;
}

/**
 * @brief       ソケットの終期化
 * @param[in]   info サーバ情報
 * @return      成功0、失敗-1
 */
void tcp_sv_socket_deinit(sv_info_t *info) {
    // 接続を閉じる
    shutdown(info->listen_sd, SHUT_RDWR);

    // ソケット破棄
    if (info->listen_sd != 0) close(info->listen_sd);

    return;
}