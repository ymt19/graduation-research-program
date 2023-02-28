#pragma once

#include <arpa/inet.h>

/*
 *  クライアントの接続先サーバ情報
 */
struct udp_client_info {
    int sd;                     /* ソケットディスクリプタ */
};
typedef struct udp_client_info udp_cl_info_t;

int udp_cl_send_msg(udp_cl_info_t *info, char *msg, int len, char *sv_ipaddr, unsigned short sv_port, char *errmsg);
int udp_cl_retrun_recv_msg(udp_cl_info_t *info, char *msg, int msg_size, char *errmsg);
int udp_cl_socket_init(udp_cl_info_t *info, int blocking, char *errmsg);
void udp_cl_socket_deinit(udp_cl_info_t *info);