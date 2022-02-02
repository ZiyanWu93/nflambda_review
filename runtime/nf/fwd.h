//
// Created by anon on 12/24/20.
//

#ifndef MULTICORE_FWDING_DPDK_FWD_H
#define MULTICORE_FWDING_DPDK_FWD_H

#include "../common.h"
#include <rte_malloc.h>
#include <rte_ethdev.h>
#include <rte_log.h>
#include <rte_timer.h>
#include "../state/state.h"

static inline void fwder_action(struct state *state, struct message *m) {
    if (state->path_instance_number[MAXIMUM_PATH -
                                    1] == 0) {
        m->action_id = -1;
    } else {
        m->action_id = state->path[MAXIMUM_PATH - 1][m->rss_hash %
                                                     state->path_instance_number[MAXIMUM_PATH -
                                                                                 1]];; // continue to the next network function
    }
}

struct state *fwder_init(struct init_state_parameter *fwd_p);

#endif //MULTICORE_FWDING_DPDK_FWD_H
