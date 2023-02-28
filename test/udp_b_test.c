#include "udp_client.h"
#include "udp_server.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define MY_PORT         9999
#define PARTNER_PORT    8888
#define MY_IPADDR                       "127.0.0.1"
#define BLOCKING 0 // ブロッキング

int main() {
    udp_sv_info_t sv_info;
    udp_cl_info_t cl_info;
    char errmsg[256];
    char msg[256];
    int msg_len;

    udp_cl_socket_init(&cl_info, errmsg);
    udp_sv_socket_init(&sv_info, MY_PORT, BLOCKING, errmsg);

    for (int i = 0; i < 10; i++) {
        printf("%d\n", i);

        // 受信
        msg_len = udp_sv_receive_msg(&sv_info, msg, 256, errmsg);
        printf("recieve:%s\n", msg);

        sleep(2);

        // 送信
        msg_len = strlen(msg);
        udp_cl_send_msg(&cl_info, msg, msg_len+1, MY_IPADDR, PARTNER_PORT, errmsg);
    }

    udp_cl_socket_deinit(&cl_info);
    udp_sv_socket_deinit(&sv_info);
}