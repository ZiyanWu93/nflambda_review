//
// Created by anon on 9/8/2021.
//

#include "nm.h"

struct network_monitor_data *network_monitor_init(struct network_monitor_parameter *nmp) {
    struct network_monitor_data *p_nmd = malloc(sizeof(struct network_monitor_data));
    p_nmd->ht_table = nmp->ht_table;
    p_nmd->value_buffer = nmp->value_buffer;
    p_nmd->threshold = nmp->threshold;
    p_nmd->core_id = nmp->core_id;
    p_nmd->interaction_counter = nmp->interaction_counter;

    p_nmd->counter = 0;

    p_nmd->temp_5tuple.proto = 0;
    p_nmd->temp_5tuple.ip_dst = 0;
    p_nmd->temp_5tuple.ip_src =0;
    p_nmd->temp_5tuple.port_src =0;
    p_nmd->temp_5tuple.port_dst =0;
    return p_nmd;
}