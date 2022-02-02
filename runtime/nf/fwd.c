//
// Created by anon on 12/24/20.
//

#include "fwd.h"

struct state *fwder_init(struct init_state_parameter *parameter){
    struct state *state = malloc(sizeof(struct state));
    memcpy(state->path, parameter->path, sizeof (int) * MAXIMUM_PATH * MAXIMUM_PATH_INSTANCE);
    return state;
}


