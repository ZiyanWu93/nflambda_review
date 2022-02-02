//
// Created by anon on 9/15/2021.
//

#include "traffic_generator.h"
struct traffic_gen_control * traffic_gen_control_init(struct traffic_gen_control_parameters tgc_p){
    struct traffic_gen_control* p_gtgc = malloc(sizeof(struct traffic_gen_control));
    p_gtgc->mempool = tgc_p.mempool;
    return p_gtgc;
}
