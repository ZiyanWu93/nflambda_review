//
// Created by anon on 9/16/2021.
//

#ifndef NF_PLATFORM_COORDINATOR_H
#define NF_PLATFORM_COORDINATOR_H

#include "zbuffer.h"
#include "common.h"


struct coordinator_data {

};

struct coordinator_control {

};


struct coordinator_data *coordinator_init();

static inline void coordinator_data_action(struct coordinator_data * coordinatorData, struct message* m) {
    DEBUG_PRINT("%s" , m->data);
//    m->event = COORDINATOR_TO_CONTROL;
    m->data = "hello world from the coordinator\n";
}

static inline void coordinator_control_action(struct coordinator_data * coordinatorControl) {

}
#endif //NF_PLATFORM_COORDINATOR_H
