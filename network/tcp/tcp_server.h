#pragma once
#include <arpa/inet.h>

/**
 * サーバ情報
 */
struct server_info {
    // 接続待ちソケットディスクリプタ
    // socket()の返り値
    int listen_sd;
    struct sockaddr_in sv_addr; // サーバのアドレス構造体
    struct sockaddr_in cl_addr; // クライアントのアドレス構造体
};
typedef struct server_info sv_info_t;

int tcp_sv_receive_msg(int connected_sd, char *buff, int len, char *errmsg);
int tcp_sv_send_msg(int connected_sd, char *sendmsg, int len, char *errmsg);
int tcp_sv_accept(sv_info_t *info, char *errmsg);
int tcp_sv_socket_init(sv_info_t *info, unsigned short sv_port,
                       int backlog, char *errmsg);
void tcp_sv_socket_deinit(sv_info_t *info);