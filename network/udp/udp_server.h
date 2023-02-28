#pragma once

#include <arpa/inet.h>

/*
 *  サーバ情報を格納する
 */
struct udp_server_info {
    int sd;                     /* ソケットディスクリプタ */
    struct sockaddr_in sv_addr; /* サーバのアドレス構造体 */

    struct sockaddr_in cl_addr;
    unsigned int cl_addr_len;
};
typedef struct udp_server_info udp_sv_info_t;

int udp_sv_receive_msg(udp_sv_info_t * info, char *msg, int msg_size, char *errmsg);
int udp_sv_return_send_msg(udp_sv_info_t *info, char *send_msg, int send_msg_len, char *errmsg);
int udp_sv_socket_init(udp_sv_info_t *info, unsigned short sv_port, int blocking, char *errmsg);
void udp_sv_socket_deinit(udp_sv_info_t *info);