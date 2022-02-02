//
// Created by anon on 10/11/21.
//

#include "dispatcher.h"

struct state *dispatcher_init(struct init_state_parameter *parameter){
    struct state *state = malloc(sizeof(struct state));
    memcpy(state->path, parameter->path, sizeof (int) * MAXIMUM_PATH * MAXIMUM_PATH_INSTANCE);
    return state;
}

