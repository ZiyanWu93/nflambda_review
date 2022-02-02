//
// Created by anon on 4/26/21.
//

#ifndef MULTICORE_FWDING_DPDK_DECOMPOSED_LB_H
#define MULTICORE_FWDING_DPDK_DECOMPOSED_LB_H

#include <rte_malloc.h>
#include <rte_ethdev.h>
#include <rte_log.h>
#include <rte_hash.h>
#include "rte_jhash.h"
#include "../common/set.h"
#include "../common/debug.h"
#include "stdint.h"
#include <rte_timer.h>

struct decomposed_LB_data
{
    int core_id;
    int queue_id;
    long rxn; // number of packets recieved
    long txn; // number of packets sent
    long rxs; // size of packets recieved
    long txs; // size of packet sent
    int max_pkt_burst;
    uint64_t start;
    volatile bool *dp_quit;

    int frequency_control;
    int complexity_data;
    int result;
    struct rte_ring *send_ring;
    struct rte_ring *recv_ring;

    // Flow Mapper-related
    struct rte_hash *ht_table_fm;

    // statistics
    long unfound;
    long found;
    long key_full;
};

struct decomposed_LB_control
{
    int core_id;
    // struct rte_timer timer;
    uint64_t start;
    volatile bool *dp_quit;
    struct rte_ring *send_ring_plist[24];
    struct rte_ring *recv_ring_plist[24];
    int n_data_component;
    long n_control_invocation;
    int complexity_control;
};

struct decomposed_LB_data_parameters
{
    int group_id; //necessary for ring identification
    int core_id;
    int queue_id;
    int max_pkt_burst;
    volatile bool *dp_quit;
    int frequency_control; // the interval of each control operatios
    int complexity_data;

    int num_entry;
    struct rte_ring *send_ring;
    struct rte_ring *recv_ring;
};

struct decomposed_LB_control_parameters
{
    int group_id; //necessary for ring identification
    int core_id;
    volatile bool *dp_quit;
    int complexity_control;
    int n_data_component;
};

struct decomposed_LB_group
{
    struct decomposed_LB_control dlb_c;
    struct decomposed_LB_data dlb_d[24];
    struct Set *data_core;
    int control_core;
    int complexity_control;
    int complexity_data;
};

struct decomposed_LB_group_parameters
{
    int group_id;
    volatile bool *dp_quit;
    struct Set *set_data_core;
    int control_core;
    int complexity_control;
    int complexity_data;

    int max_pkt_burst;
    int frequency_control; // the interval of each control operatios

    int num_entry;

    struct Set *queue_set;
};

int decomposed_LB_data_action(struct decomposed_LB_data *p_dlb_d);
void decomposed_LB_data_init(struct decomposed_LB_data *p_dlb_d, struct decomposed_LB_data_parameters *p_dlb_d_parameter);
void decomposed_LB_data_destroy(struct decomposed_LB_data *p_dlb_d);
void decomposed_LB_data_create(struct decomposed_LB_data *p_dlb_d, int group_id, int data_no, struct decomposed_LB_control *p_dlb_c);

int decomposed_LB_control_action(struct decomposed_LB_control *dlb_c);
void decomposed_LB_control_init(struct decomposed_LB_control *dlb_c, struct decomposed_LB_control_parameters *p_p);
void decomposed_LB_control_destroy(struct decomposed_LB_control *dlb_c);
void decomposed_LB_control_create(struct decomposed_LB_control *glb_c, int group_id);

void decomposed_LB_group_init(struct decomposed_LB_group *pp_dlb_p, struct decomposed_LB_group_parameters *p_parameter_dlb_m);
void decomposed_LB_group_destroy(struct decomposed_LB_group *pp_dlb_p);
void decomposed_LB_group_create(struct decomposed_LB_group *pp_dlb_p, int group_id);
#endif //MULTICORE_FWDING_DPDK_DECOMPOSED_LB_H
