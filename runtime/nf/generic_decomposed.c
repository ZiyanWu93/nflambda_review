#include "generic_decomposed.h"
#include "../common/defined.h"
#include <rte_hash.h>
#include "rte_jhash.h"

struct state *
generic_decomposed_data_init(struct init_state_parameter *parameter) {
    //DEBUG_PRINT("generic_decomposed_data_init start\n");
    // *state = malloc(sizeof(struct generic_decomposed_data));
    struct state *state = malloc(sizeof(struct state));
    (*state).data_complexity = parameter->data_complexity;
    (*state).control_frequency = parameter->control_frequency;
    (*state).counter = 0;

    char s[100];
    snprintf(s, sizeof(s), "hash %d", parameter->execution_id);
    // RTE_HASH_EXTRA_FLAGS_MULTI_WRITER_ADD
    struct rte_hash_parameters ht_table_type_fm = {
            .name = s,
            .entries = parameter->state_size,
            .key_len = sizeof(struct ipv4_5tuple),
            .hash_func = rte_jhash,
            .hash_func_init_val = 0,
            .socket_id = 1,
    };
    state->hash_table = rte_hash_create(&ht_table_type_fm);

    state->rand_range = parameter->data_state_size;

    (*state).key.ip_src = 0;
    (*state).key.ip_dst = 0;
    (*state).key.port_dst = 0;
    (*state).key.port_src = parameter->execution_id;
    (*state).key.proto = 0;

    (*state).n_message = 0;
    memcpy(state->path, parameter->path, sizeof(int) * MAXIMUM_PATH * MAXIMUM_PATH_INSTANCE);
    return state;
    //DEBUG_PRINT("generic_decomposed_data_init done\n");
}


struct state *generic_decomposed_control_init(
        struct init_state_parameter *parameter) {
    struct state *state = malloc(sizeof(struct state));
    //DEBUG_PRINT("generic_decomposed_control_init start\n");
    char s[64];
    unsigned flags = 0;
    unsigned ring_size = 64;

    (*state).control_complexity = parameter->control_complexity;
    snprintf(s, sizeof(s), "hash %d", parameter->execution_id);
    // RTE_HASH_EXTRA_FLAGS_MULTI_WRITER_ADD
    struct rte_hash_parameters ht_table_type_fm = {
            .name = s,
            .entries = parameter->state_size,
            .key_len = sizeof(struct ipv4_5tuple),
            .hash_func = rte_jhash,
            .hash_func_init_val = 0,
            .socket_id = 1,
    };
    state->hash_table = rte_hash_create(&ht_table_type_fm);
    (*state).rand_range = parameter->control_state_size;

    (*state).key.ip_src = 0;
    (*state).key.ip_dst = 0;
    (*state).key.port_dst = 0;
    (*state).key.port_src = parameter->execution_id;
    (*state).key.proto = 0;
    (*state).n_message = 0;
    memcpy(state->path, parameter->path, sizeof(int) * MAXIMUM_PATH * MAXIMUM_PATH_INSTANCE);
    //DEBUG_PRINT("generic_decomposed_control_init done\n");
    return state;
}

struct state *generic_decomposed_control_shared_init(
        struct init_state_parameter *parameter) {
    struct state *state = malloc(sizeof(struct state));
    DEBUG_PRINT("generic_decomposed_control_init start\n");
    char s[64];
    (*state).rand_range = parameter->control_state_size;
    (*state).control_complexity = parameter->control_complexity;
    snprintf(s, sizeof(s), "hash %d", parameter->execution_id);
    // RTE_HASH_EXTRA_FLAGS_MULTI_WRITER_ADD
    state->hash_table = parameter->shared_state;

    (*state).key.ip_src = 0;
    (*state).key.ip_dst = 0;
    (*state).key.port_dst = 0;
    (*state).key.port_src = parameter->execution_id;
    (*state).key.proto = 0;
    (*state).n_message = 0;
    memcpy(state->path, parameter->path, sizeof(int) * MAXIMUM_PATH * MAXIMUM_PATH_INSTANCE);
    DEBUG_PRINT("generic_decomposed_control_init done\n");

    return state;
}
