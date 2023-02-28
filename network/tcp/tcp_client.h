#pragma once
#include <arpa/inet.h>

/**
 * クライアントの接続先サーバ情報
 */
struct client_info {
    int sd;                     // ソケットディスクリプタ
    struct sockaddr_in sv_addr; // サーバのアドレス構造体
};
typedef struct client_info cl_info_t;

int tcp_cl_receive_msg(cl_info_t *info, char *recvmsg, int len, char *errmsg);
int tcp_cl_send_msg(cl_info_t *info, char* sendmsg, int len, char *errmsg);
int tcp_cl_connect(cl_info_t *info, char *errmsg);
int tcp_cl_socket_init(cl_info_t *info, char *sv_ipaddr, unsigned short sv_port, char *errmsg);
void tcp_cl_socket_deinit(cl_info_t *info);
