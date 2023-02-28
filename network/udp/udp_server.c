#include "../../network/udp/udp_server.h"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/**
 * @brief メッセージを受信
 * 
 * @param info 
 * @param msg 
 * @param msg_size 
 * @param errmsg 
 * @return int
 * @note ノンブロッキングの場合、受信しなかったらerrnoにEAGAINをセットし、-1を返す
 */
int udp_sv_receive_msg(udp_sv_info_t * info, char *msg, int msg_size, char *errmsg) {
    int recv_msglen = 0;
    // struct sockaddr_in cl_addr = {0};
    // unsigned int cl_addr_len = 0;
    info->cl_addr_len = sizeof(info->cl_addr);

    // cl_addr_len = sizeof(cl_addr);
    // recv_msglen = recv(info->sd, msg, msg_size, 0);
    recv_msglen = recvfrom(info->sd, msg, msg_size, 0,
                    (struct sockaddr *)&(info->cl_addr), &(info->cl_addr_len));
    if(recv_msglen < 0){
        sprintf(errmsg, "(line:%d) %s", __LINE__, strerror(errno));
        return -1;
    }

    return recv_msglen;
}

/**
 * @brief 直前に受信したメッセージ送信者にメッセージを送信
 * 
 * @param info 
 * @param send_msg 
 * @param send_msg_len 
 * @param errmsg 
 * @return int 
 */
int udp_sv_return_send_msg(udp_sv_info_t *info, char *send_msg, int send_msg_len, char *errmsg) {
    int rc = 0;

    rc = sendto(info->sd, send_msg, send_msg_len, 0, 
                (struct sockaddr *)&(info->cl_addr), info->cl_addr_len);
    if (rc != send_msg_len) {
        sprintf(errmsg, "(line:%d) %s", __LINE__, strerror(errno));
        return -1;
    }

    return rc;
}

/**
 * @brief UDPサーバソケットの初期化
 * 
 * @param info サーバ情報
 * @param sv_port サーバポート番号
 * @param blocking ブロッキング設定(0:ブロッキング、0以外:ノンブロッキング)
 * @param errmsg 
 * @return int 
 */
int udp_sv_socket_init(udp_sv_info_t *info, unsigned short sv_port, int blocking, char *errmsg) {
    int rc = 0;

    /* ソケットの生成 : UDPを指定する */
    info->sd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(info->sd < 0){
        sprintf(errmsg, "(line:%d) %s", __LINE__, strerror(errno));
        return -1;
    }

    /* サーバのアドレス構造体を作成する */
    info->sv_addr.sin_family = AF_INET;
    info->sv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    info->sv_addr.sin_port = htons(sv_port);

    /* ローカルアドレスへバインドする */
    rc = bind(info->sd, (struct sockaddr *)&(info->sv_addr), sizeof(info->sv_addr));
    if(rc != 0){
        sprintf(errmsg, "(line:%d) %s", __LINE__, strerror(errno));
        return -1;
    }

    // ブロッキング設定
    rc = ioctl(info->sd, FIONBIO, &blocking);
    if(rc == -1){
        sprintf(errmsg, "(line:%d) %s", __LINE__, strerror(errno));
        return -1;
    }

    return 0;
}

/*!
 * @brief      ソケットの終期化
 * @param[in]  info   クライアント接続情報
 * @return     成功ならば0、失敗ならば-1を返す。
 */
void udp_sv_socket_deinit(udp_sv_info_t *info) {
    /* ソケット破棄 */
    if(info->sd != 0) close(info->sd);
}