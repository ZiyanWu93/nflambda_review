//
// Created by anon on 12/21/2021.
//

#include "state.h"


struct state * state_init(struct init_state_parameter *parameter) {
    DEBUG_PRINT("generic_decomposed_data_init start\n");
    // *state = malloc(sizeof(struct generic_decomposed_data));
    struct state *state = malloc(sizeof(struct state));


    char s[100];
    snprintf(s, sizeof(s), "hash %d", parameter->execution_id);
    // RTE_HASH_EXTRA_FLAGS_MULTI_WRITER_ADD
    struct rte_hash_parameters ht_table_type_fm = {
            .name = s,
            .entries = 200000,
            .key_len = sizeof(struct ipv4_5tuple),
                    .hash_func = rte_jhash,
                    .hash_func_init_val = 0,
                    .socket_id = 1,
                    };


    state->hash_table = rte_hash_create(&ht_table_type_fm);

    memcpy(state->path, parameter->path, sizeof(int) * MAXIMUM_PATH * MAXIMUM_PATH_INSTANCE);
    return state;
    DEBUG_PRINT("generic_decomposed_data_init done\n");
}

struct state * state_update(struct state* state, struct init_state_parameter *parameter){
    state->rand_range = parameter->data_state_size;
    state->control_frequency = parameter->control_frequency;
    state->control_complexity = parameter->control_complexity;
    state->data_complexity = parameter->data_complexity;
    state->counter = 0;
}