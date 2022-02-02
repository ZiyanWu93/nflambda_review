//
// Created by anon on 4/24/21.
//

#ifndef MULTICORE_FWDING_DPDK_MONO_LB_H
#define MULTICORE_FWDING_DPDK_MONO_LB_H

#include <rte_malloc.h>
#include <rte_ethdev.h>
#include <rte_log.h>
#include <rte_timer.h>
#include "../common/defined.h"

struct service_selector {
    struct server_ip_map serverIPMAP;
    int index;
};

struct mono_lb {
    int core_id;
    int queue_id;
    long rxn; // number of packets recieved
    long txn; // number of packets sent
    long rxs; // size of packets recieved
    long txs; // size of packet sent
    long key_full;
    int max_pkt_burst;
    struct rte_timer timer;
    uint64_t start;
    volatile bool *dp_quit;

    // Flow Mapper-related
    struct rte_hash *ht_table_fm;
    // Server Selector-related
    rte_atomic32_t server_ip_index;
    int table_size;
    struct rte_hash *ht_table_ss;

    struct lb_monolithic_options *opts;

    long found;
    long unfound;
    int last_port;
};


int mono_lb_action(struct mono_lb *monolb);
void mono_lb_create(struct mono_lb *p_lb, int id);
void mono_lb_init(struct mono_lb *p_lb, int queue_id, int lcore_id, int max_pkt_burst, volatile bool *dp_quit);
void mono_lb_destroy(struct mono_lb **p_lb);

#endif //MULTICORE_FWDING_DPDK_MONO_LB_H
