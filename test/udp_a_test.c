#include "udp_client.h"
#include "udp_server.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define MY_PORT         8888
#define PARTNER_PORT    9999
#define MY_IPADDR                       "127.0.0.1"
#define BLOCKING 1 // ノンブロッキング

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

        // 送信
        sprintf(msg, "message%d", i);
        msg_len = strlen(msg);
        udp_cl_send_msg(&cl_info, msg, msg_len+1, MY_IPADDR, PARTNER_PORT, errmsg);


        while (1) {
            msg_len = udp_sv_receive_msg(&sv_info, msg, 256, errmsg);
            if (msg_len < 0) {
                if (errno == EAGAIN) {
                    printf("yet len:%d\n", msg_len);
                }
            } else {
                printf("recieve:%s\n", msg);
                break;
            }
            sleep(1);
        }
    }

    udp_cl_socket_deinit(&cl_info);
    udp_sv_socket_deinit(&sv_info);
}