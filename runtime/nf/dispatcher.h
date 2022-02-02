//
// Created by anon on 10/11/21.
//

#ifndef NF_PLATFORM_DISPATCHER_H
#define NF_PLATFORM_DISPATCHER_H

#include "../common.h"
#include "../state/state.h"

static inline void dispatcher_action(struct state *state, struct message *m) {
    if (state->path_instance_number[MAXIMUM_PATH -
                                    1] == 0) {
        m->action_id = -1;
    } else {
        m->action_id = state->path[MAXIMUM_PATH - 1][m->rss_hash %
                                                     state->path_instance_number[MAXIMUM_PATH -
                                                                                 1]];; // continue to the next network function
    }
}

struct state *dispatcher_init(struct init_state_parameter *parameter);

#endif //NF_PLATFORM_DISPATCHER_H
