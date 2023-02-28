#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../utils/configuration.h"
#include "../utils/timer.h"

/**
 * @brief       configのメモリを確保し値を設定
 * @param       system_duration
 * @param       epoch_duration
 * @param       replica_num マスタノードを含めたレプリカ数
 * @param       my_replica_id
 */
Config *create_config(exp_t exp, double epoch_duration, int my_replica_id) {
    Config *config;
    config = malloc(sizeof(Config));
    config->exp = exp;
    // config->system_duration = system_duration;
    // config->system_startup_time = -1;
    config->system_finish_request = 0;
    config->epoch_duration = epoch_duration;
    config->my_replica_id = my_replica_id;

    /*********** node info *************/
    // config->node_info[0].region_id = 0;
    // config->node_info[1].region_id = 1;
    // config->node_info[2].region_id = 1;
    // config->node_info[3].region_id = 1;
    // config->node_info[4].region_id = 0;
    // config->node_info[5].region_id = 0;
    // config->node_info[6].region_id = 0;
    /*********** node info *************/

    config->storage = new_storage(STORAGE_SIZE);
    config->batch_log = create_log_info(BATCH_LOG_FILENAME);
    config->write_tx_buffer = create_batch_list();
    config->read_tx_buffer = create_tx_list();

    sprintf(config->log_filename, "log_%d.csv", my_replica_id);
    pthread_mutex_init(&config->log_fp_mutex, NULL);
    config->log_fp = fopen(config->log_filename, "w");

    // sprintf(config->send_log_filename, "send_log_%d.csv", my_replica_id);
    // pthread_mutex_init(&config->send_log_fp_mutex, NULL);
    // config->send_log_fp = fopen(config->send_log_filename, "w");
    // // if (config->send_log_fp == NULL) {
    // //     return -1;
    // // }

    // sprintf(config->recv_log_filename, "recv_log_%d.csv", my_replica_id);
    // pthread_mutex_init(&config->recv_log_fp_mutex, NULL);
    // config->recv_log_fp = fopen(config->recv_log_filename, "w");
    // // if (config->recv_log_fp == NULL) {
    // //     return -1;
    // // }

    return config;
}

/**
 * @brief Set the system start time object
 *
 * @param config
 */
// void set_system_start_time(Config *config) {
//     config->system_startup_time = get_time();
// }

/**
 * @brief Set the system finish request object
 *
 * @param config
 */
void set_system_finish_request(Config *config) {
    config->system_finish_request = 1;
}

/**
 * @brief       システム終了時刻を過ぎているか確認
 * @param       config
 * @param       now
 * @return      終了時刻超過なら1、そうでないなら0
 * @note
 * シーケンサのエポック開始時に呼び出し、fisnish requestを要請
 */
// int check_system_finish_time(Config *config, double now) {
//     if (now >= config->system_duration + config->system_startup_time) {
//         set_system_finish_request(config);
//     }
//     return config->system_finish_request;
// }

/**
 * @brief       configのメモリを解放
 */
void free_config(Config *config) {
    free_storage(config->storage);
    free_batch_list(config->write_tx_buffer);
    free_tx_list(config->read_tx_buffer);
    free_log_info(config->batch_log);
    fclose(config->log_fp);
    pthread_mutex_destroy(&config->log_fp_mutex);
    // fclose(config->send_log_fp);
    // pthread_mutex_destroy(&config->send_log_fp_mutex);
    // fclose(config->recv_log_fp);
    // pthread_mutex_destroy(&config->recv_log_fp_mutex);
    free(config);
}

int get_port_master_sequencer_from_client_interface(int replica_id,
                                                    int client_id) {
    // return PORT_MASTER_SEQUENCER_FROM_CLIENT_INTERFACE;
    if (replica_id == 1) {
        return PORT_MASTER_SEQUENCER_FROM_SLAVE1_CLIENT_INTERFACE + client_id;
    } else if (replica_id == 2) {
        return PORT_MASTER_SEQUENCER_FROM_SLAVE2_CLIENT_INTERFACE + client_id;
    } else if (replica_id == 3) {
        return PORT_MASTER_SEQUENCER_FROM_SLAVE3_CLIENT_INTERFACE + client_id;
    } else if (replica_id == 4) {
        return PORT_MASTER_SEQUENCER_FROM_SLAVE4_CLIENT_INTERFACE + client_id;
    } else if (replica_id == 5) {
        return PORT_MASTER_SEQUENCER_FROM_SLAVE5_CLIENT_INTERFACE + client_id;
    } else if (replica_id == 6) {
        return PORT_MASTER_SEQUENCER_FROM_SLAVE6_CLIENT_INTERFACE + client_id;
    }
}

int get_port_master_sequencer_from_slave_sequencer(int replica_id) {
    if (replica_id == 1) {
        return PORT_MASTER_SEQUENCER_FROM_SLAVE1_SEQUENCER;
    } else if (replica_id == 2) {
        return PORT_MASTER_SEQUENCER_FROM_SLAVE2_SEQUENCER;
    } else if (replica_id == 3) {
        return PORT_MASTER_SEQUENCER_FROM_SLAVE3_SEQUENCER;
    } else if (replica_id == 4) {
        return PORT_MASTER_SEQUENCER_FROM_SLAVE4_SEQUENCER;
    } else if (replica_id == 5) {
        return PORT_MASTER_SEQUENCER_FROM_SLAVE5_SEQUENCER;
    } else if (replica_id == 6) {
        return PORT_MASTER_SEQUENCER_FROM_SLAVE6_SEQUENCER;
    }
}

int get_port_slave_sequencer(int replica_id) {
    if (replica_id == 1) {
        return PORT_SLAVE1_SEQUENCER;
    } else if (replica_id == 2) {
        return PORT_SLAVE2_SEQUENCER;
    } else if (replica_id == 3) {
        return PORT_SLAVE3_SEQUENCER;
    } else if (replica_id == 4) {
        return PORT_SLAVE4_SEQUENCER;
    } else if (replica_id == 5) {
        return PORT_SLAVE5_SEQUENCER;
    } else if (replica_id == 6) {
        return PORT_SLAVE6_SEQUENCER;
    }
}

int get_port_slave_client_interface_from_client(int replica_id) {
    if (replica_id == 1) {
        return PORT_SLAVE1_CLINET_INTERFACE_FROM_CLIENT;
    } else if (replica_id == 2) {
        return PORT_SLAVE2_CLINET_INTERFACE_FROM_CLIENT;
    } else if (replica_id == 3) {
        return PORT_SLAVE3_CLINET_INTERFACE_FROM_CLIENT;
    } else if (replica_id == 4) {
        return PORT_SLAVE4_CLINET_INTERFACE_FROM_CLIENT;
    } else if (replica_id == 5) {
        return PORT_SLAVE5_CLINET_INTERFACE_FROM_CLIENT;
    } else if (replica_id == 6) {
        return PORT_SLAVE6_CLINET_INTERFACE_FROM_CLIENT;
    }
}

int get_port_finish_from_client(int replica_id) {
    if (replica_id == 0) {
        return PORT_FINISH_MASTER_FROM_CLIENT;
    } else if (replica_id == 1) {
        return PORT_FINISH_SLAVE1_FROM_CLIENT;
    } else if (replica_id == 2) {
        return PORT_FINISH_SLAVE2_FROM_CLIENT;
    } else if (replica_id == 3) {
        return PORT_FINISH_SLAVE3_FROM_CLIENT;
    } else if (replica_id == 4) {
        return PORT_FINISH_SLAVE4_FROM_CLIENT;
    } else if (replica_id == 5) {
        return PORT_FINISH_SLAVE5_FROM_CLIENT;
    } else if (replica_id == 6) {
        return PORT_FINISH_SLAVE6_FROM_CLIENT;
    }
}

// int get_port_slave_client_interface_from_master_sequencer(int replica_id, int
// client_id) {
//     if (replica_id == 1) {
//         return PORT_SLAVE1_CLIENT_INTERFACE_FROM_MASTER_SEQUENCER+client_id;
//     } else if (replica_id == 2) {
//         return PORT_SLAVE2_CLIENT_INTERFACE_FROM_MASTER_SEQUENCER+client_id;
//     } else if (replica_id == 3) {
//         return PORT_SLAVE3_CLIENT_INTERFACE_FROM_MASTER_SEQUENCER+client_id;
//     } else if (replica_id == 4) {
//         return PORT_SLAVE4_CLIENT_INTERFACE_FROM_MASTER_SEQUENCER+client_id;
//     } else if (replica_id == 5) {
//         return PORT_SLAVE5_CLIENT_INTERFACE_FROM_MASTER_SEQUENCER+client_id;
//     } else if (replica_id == 6) {
//         return PORT_SLAVE6_CLIENT_INTERFACE_FROM_MASTER_SEQUENCER+client_id;
//     }
// }

void get_sequencer_ipaddr(int replica_id, char *ip_addr) {
    /*** 1PCのみの実験環境  ***/
    // strcpy(ip_addr, MY_IPADDR);
    // return;

    if (replica_id == 0) {
        strcpy(ip_addr, MASTER_SEQUENCER_IPADDR);
    } else if (replica_id == 1) {
        strcpy(ip_addr, SLAVE1_SEQUENCER_IPADDR);
    } else if (replica_id == 2) {
        strcpy(ip_addr, SLAVE2_SEQUENCER_IPADDR);
    } else if (replica_id == 3) {
        strcpy(ip_addr, SLAVE3_SEQUENCER_IPADDR);
    } else if (replica_id == 4) {
        strcpy(ip_addr, SLAVE4_SEQUENCER_IPADDR);
    } else if (replica_id == 5) {
        strcpy(ip_addr, SLAVE5_SEQUENCER_IPADDR);
    } else if (replica_id == 6) {
        strcpy(ip_addr, SLAVE6_SEQUENCER_IPADDR);
    }
}

void get_client_interface_ipaddr(int replica_id, char *ip_addr) {
    /*** 1PCのみの実験環境  ***/
    // strcpy(ip_addr, MY_IPADDR);
    // return;

    if (replica_id == 0) {
        strcpy(ip_addr, MASTER_CLIENT_INTERFACE_IPADDR);
    } else if (replica_id == 1) {
        strcpy(ip_addr, SLAVE1_CLIENT_INTERFACE_IPADDR);
    } else if (replica_id == 2) {
        strcpy(ip_addr, SLAVE2_CLIENT_INTERFACE_IPADDR);
    } else if (replica_id == 3) {
        strcpy(ip_addr, SLAVE3_CLIENT_INTERFACE_IPADDR);
    } else if (replica_id == 4) {
        strcpy(ip_addr, SLAVE4_CLIENT_INTERFACE_IPADDR);
    } else if (replica_id == 5) {
        strcpy(ip_addr, SLAVE5_CLIENT_INTERFACE_IPADDR);
    } else if (replica_id == 6) {
        strcpy(ip_addr, SLAVE6_CLIENT_INTERFACE_IPADDR);
    }
}

void get_within_region_slave_ipaddr(int replica_id, char *ip_addr) {
    /*** 1PCのみの実験環境  ***/
    // strcpy(ip_addr, MY_IPADDR);
    // return;

    if (replica_id == 1) {
        strcpy(ip_addr, WITHIN_REGION_SLAVE1_IPADDR);
    } else if (replica_id == 2) {
        strcpy(ip_addr, WITHIN_REGION_SLAVE2_IPADDR);
    } else if (replica_id == 3) {
        strcpy(ip_addr, WITHIN_REGION_SLAVE3_IPADDR);
    } else if (replica_id == 4) {
        strcpy(ip_addr, WITHIN_REGION_SLAVE4_IPADDR);
    } else if (replica_id == 5) {
        strcpy(ip_addr, WITHIN_REGION_SLAVE5_IPADDR);
    } else if (replica_id == 6) {
        strcpy(ip_addr, WITHIN_REGION_SLAVE6_IPADDR);
    }
}

/**
 * @brief Get the rto object
 *
 * @param replica_id
 * @return double
 */
double get_rto(int replica_id) {
    double rto;
    if (replica_id == 1) {
        rto = RTO_SLAVE1;
    } else if (replica_id == 2) {
        rto = RTO_SLAVE2;
    } else if (replica_id == 3) {
        rto = RTO_SLAVE3;
    } else if (replica_id == 4) {
        rto = RTO_SLAVE4;
    } else if (replica_id == 5) {
        rto = RTO_SLAVE5;
    } else if (replica_id == 6) {
        rto = RTO_SLAVE6;
    }
    return rto;
}

/**
 * @brief Get the region object
 *
 * @param replica_id
 * @return int
 */
int get_region(int replica_id) {
    int region;
    if (replica_id == 1) {
        region = REGION_SLAVE1;
    } else if (replica_id == 2) {
        region = REGION_SLAVE2;
    } else if (replica_id == 3) {
        region = REGION_SLAVE3;
    } else if (replica_id == 4) {
        region = REGION_SLAVE4;
    } else if (replica_id == 5) {
        region = REGION_SLAVE5;
    } else if (replica_id == 6) {
        region = REGION_SLAVE6;
    }
    return region;
}