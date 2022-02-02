//
// Created by anon on 5/3/21.
//

#ifndef MULTICORE_FWDING_DPDK_SS_LB_H
#define MULTICORE_FWDING_DPDK_SS_LB_H

#include <rte_malloc.h>
#include <rte_ethdev.h>
#include <rte_log.h>
#include <rte_timer.h>
#include <rte_hash.h>
#include "rte_jhash.h"
#include "../common/defined.h"
#include "stdbool.h"

struct lb_data {
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
    struct service_selector *associated_service_selector;
    struct rte_ring *send_ring, *recv_ring;
    long found;
    long unfound;
    int last_port;
};

struct lb_data_parameters {
    unsigned lcore_id;
    int queue_id;
    int max_pkt_burst;
    int num_entry;
    volatile bool *dp_quit;
    struct rte_ring *send_ring;
    struct rte_ring *recv_ring;
};


struct lb_control_ss {
    struct server_ip_map serverIPMAP;
    int index;

    volatile bool *dp_quit;
    struct rte_ring **send_ring_plist;
    struct rte_ring **recv_ring_plist;
    struct lb_data **lbData;
    int n_data_component;

    long control_count;
    bool running;
};


int lb_data_action(struct lb_data *lb_data);

void lb_data_init(struct lb_data **p_lb_data, struct lb_data_parameters lbDataParameters);

void lb_control_ss_action(struct lb_control_ss *lbControlSss);

void lb_control_ss_init(struct lb_control_ss **p_lb_control, int n_data_component, int max_pkt_burst,
                        volatile bool *dp_quit);

#endif //MULTICORE_FWDING_DPDK_SS_LB_H
