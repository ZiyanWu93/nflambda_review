#ifndef NF_GENERIC_DECOMPOSED_H
#define NF_GENERIC_DECOMPOSED_H

#include <rte_malloc.h>
#include <rte_ethdev.h>
#include <rte_log.h>
#include "../common/set.h"
#include "../common/debug.h"
#include "../common.h"
#include "stdint.h"
#include <rte_timer.h>
#include <rte_hash.h>
#include "../state/state.h"

struct state *generic_decomposed_data_init(struct init_state_parameter *parameter);

struct state *generic_decomposed_control_init(struct init_state_parameter *parameter);


static inline void generic_decomposed_classifier(struct state *state, struct message *m) {
//    DEBUG_PRINT("generic_decomposed_classifier\n");
    if (state->control_frequency == 0) {
        m->path = state->simple_path[0];
        return;
    }
    if (state->control_frequency > 0) {
        if (state->control_frequency == state->counter) {
            state->counter = 0;
//            state->counter = state->path[1];
            //m->action_id = state->path[0][m->rss_hash % state->path_instance_number[0]]; // go to the control path
            m->path = state->simple_path[1];
        } else {
            state->counter += 1;
            //m->action_id = state->path[1][m->rss_hash % state->path_instance_number[1]]; // go to the data path
            m->path = state->simple_path[0];
        }
    } else {
        if (state->control_frequency == state->counter) {
            state->counter = 0;
            //m->action_id = state->path[1][m->rss_hash % state->path_instance_number[1]]; // go to the data path
            m->path = state->simple_path[0];;
        } else {
            state->counter -= 1;
            //m->action_id = state->path[0][m->rss_hash % state->path_instance_number[0]]; // go to the control path
            m->path = state->simple_path[1];
        }
    }
}


static inline void
generic_decomposed_control_action(struct state *state, struct message *m) {
    //DEBUG_PRINT("control action %d\n", m->action_id);
    operation(state->control_complexity);
    // access control state
    state->key.ip_src = (state->key.ip_src + 1) % (state->rand_range);
    int ret = rte_hash_lookup((*state).hash_table, &state->key);
    if (ret == -ENOENT) { //  not found
        ret = rte_hash_add_key((*state).hash_table, &state->key);
        if (ret >= 0)
            state->value[ret] = 1;
    } else { // found
        state->value[ret] += 1;
    }

    //m->action_id = state->path[0][m->rss_hash % state->path_instance_number[0]];
    m->path = state->simple_path[0];
    state->hash_table_size = rte_hash_count(state->hash_table);
    state->n_message++;
};

static inline void
generic_decomposed_data_action(struct state *state, struct message *m) {
    //("data action %d\n", m->action_id);
    operation(state->data_complexity);
    state->key.ip_src = (state->key.ip_src + 1) % (state->rand_range);
//     access hash_table
    int ret = rte_hash_lookup((*state).hash_table, &state->key);
    if (ret == -ENOENT) { // not found
        ret = rte_hash_add_key((*state).hash_table, &state->key);
        state->value[ret] = 1;
    } else { // found
        state->value[ret] += 1;
    }

    state->hash_table_size = rte_hash_count(state->hash_table);
    m->path = state->simple_path[2];
    state->n_message++;
};


static inline void generic_decomposed_parallel_data_action(struct state *state, struct message *m) {
    operation(state->data_complexity);
    struct ipv4_5tuple temp_5tuple = {
            .ip_src = rand() % state->rand_range,
            .ip_dst = 0,
            .port_dst =0,
            .port_src = 0,
            .proto = 0
    };
    // access hash_table
    int ret = rte_hash_lookup((*state).hash_table, &temp_5tuple);
    if (ret != -ENOENT) { //  not found
        ret = rte_hash_add_key((*state).hash_table, &temp_5tuple);
        state->value[ret] = 1;
    } else { // found
        state->value[ret] += 1;
    }

    m->path = 0b01; // continue to the next network function
};

static inline void
generic_decomposed_parallel_control_action(struct state *gdc_state, struct message *m) {
    operation(gdc_state->control_complexity);
    // access control
    m->path = 0b01; // continue to the data action
};

static inline void
generic_decomposed_parallel_classifier(struct state *gdd_state, struct message *m) {
    if (gdd_state->control_frequency == gdd_state->counter) {
        gdd_state->counter = 0;
        m->path = gdd_state->path[0][m->rss_hash %
                                     gdd_state->path_instance_number[0]]; // go to the control path and the data path
    } else {
        gdd_state->counter += 1;
        m->path = gdd_state->path[1][m->rss_hash % gdd_state->path_instance_number[1]]; // // go to the data path
    }
}

extern void (*monitor_action_list[2])(void *, struct message *);

struct state *generic_decomposed_control_shared_init(
        struct init_state_parameter *parameter);

#endif /* NF_GENERIC_DECOMPOSED_H */
