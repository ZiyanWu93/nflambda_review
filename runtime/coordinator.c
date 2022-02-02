//
// Created by anon on 9/16/2021.
//

#include "coordinator.h"
struct coordinator_data *coordinator_init() {
    struct coordinator_data *p_cd = malloc(sizeof(struct coordinator_data));
    return p_cd;
}