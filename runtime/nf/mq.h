//
// Created by anon on 5/2/21.
//

#ifndef MULTICORE_FWDING_DPDK_MP_H
#define MULTICORE_FWDING_DPDK_MP_H
#include "../Config.h"
struct Config;
struct mq {
    int core_id;
    struct rte_ring *send_ring, *recv_ring;
    struct mq* mq_peer;
};

void mq_action(struct mq * p_mq);
void mq_init(struct Config *c);
#endif //MULTICORE_FWDING_DPDK_MP_H
