#include "../../network/udp/udp_client.h"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/*!
 * @brief      UDP接続してメッセージを送る
 * @param[in]  info   クライアント接続情報
 * @param[out] errmsg エラーメッセージ格納先
 * @return     成功ならば送信メッセージ長、失敗ならば-1を返す。
 */
int udp_cl_send_msg(udp_cl_info_t *info, char *msg, int len, char *sv_ipaddr, unsigned short sv_port, char *errmsg) {
    int rc = 0;
    struct sockaddr_in sv_addr;

    /* サーバのアドレス構造体を作成する */
    sv_addr.sin_family = AF_INET;
    sv_addr.sin_addr.s_addr = inet_addr(sv_ipaddr);
    sv_addr.sin_port = htons(sv_port);

    /* メッセージの送信 */
    rc = sendto(info->sd, msg, len, 0, (struct sockaddr *)&sv_addr, sizeof(sv_addr));
    if(rc != len){
        sprintf(errmsg, "(line:%d) %s", __LINE__, strerror(errno));
        return -1;
    }

    return rc;
}

/**
 * @brief 
 * 
 * @param info 
 * @param msg 
 * @param msg_size 
 * @param errmsg 
 * @return int 
 */
int udp_cl_retrun_recv_msg(udp_cl_info_t *info, char *msg, int msg_size, char *errmsg) {
    int recv_msg_len = 0;
    struct sockaddr_in sv_addr = {0};
    unsigned int sv_addr_len = 0;

    sv_addr_len = sizeof(sv_addr);
    recv_msg_len = recvfrom(info->sd, msg, msg_size, 0, 
                            (struct sockaddr *)&sv_addr, &sv_addr_len);
    if(recv_msg_len < 0){
        sprintf(errmsg, "(line:%d) %s", __LINE__, strerror(errno));
        return -1;
    }

    return recv_msg_len;
}

/**
 * @brief 
 * 
 * @param info 
 * @param blocking ブロッキング設定(0:ブロッキング、0以外:ノンブロッキング)
 * @param errmsg 
 * @return int 
 */
int udp_cl_socket_init(udp_cl_info_t *info, int blocking, char *errmsg) {
    int rc;

    /* ソケットの生成 : UDPを指定する */
    info->sd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(info->sd < 0){
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
void udp_cl_socket_deinit(udp_cl_info_t *info) {
    /* ソケット破棄 */
    if(info->sd != 0) close(info->sd);
}