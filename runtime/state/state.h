//
// Created by anon on 12/21/2021.
//

#ifndef NF_PLATFORM_STATE_H
#define NF_PLATFORM_STATE_H

#include "../common.h"
#include "rte_jhash.h"
#include "rte_hash.h"

struct state {
    struct rte_hash *hash_table;
    int data_complexity;
    int control_complexity;
    int control_frequency;
    int counter;
    struct ipv4_5tuple key;
    int rand_range;
    int value[1000000];
    int path[MAXIMUM_PATH][MAXIMUM_PATH_INSTANCE];
    int path_instance_number[MAXIMUM_PATH];

    int simple_path[10]; // action_id
    int n_message;
    int hash_table_size;
    /// 10 possible paths, each path can have up to 24 instances
    /// the last path (path[9][]) is used for the subsequent network functions
};

struct state * state_init(struct init_state_parameter *parameter);

struct state * state_update(struct state* state, struct init_state_parameter *parameter);
#endif //NF_PLATFORM_STATE_H