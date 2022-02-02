//
// Created by anon on 5/5/21.
//

#ifndef NF_PLATFORM_PORT_H
#define NF_PLATFORM_PORT_H

#include <rte_ethdev.h>
#include <rte_common.h>
#include <rte_log.h>
#include <rte_launch.h>
#include <rte_lcore.h>
#include <rte_timer.h>
#include "Config.h"

struct port_parameter;
struct Config;
struct port_parameter {
    int port;
    int n_queue;
    int num_mbufs;
    int mbuf_cache_size;
    uint16_t nb_rxd;
    uint16_t nb_txd;

    struct Set *queue_set;
    struct rte_mempool *mempool;
};

int port_init(struct port_parameter *portParameter);

#endif //NF_PLATFORM_PORT_H
