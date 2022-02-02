//
// Created by anon on 9/8/2021.
//

#ifndef NF_PLATFORM_NM_H
#define NF_PLATFORM_NM_H

#include "../common.h"
#include "rte_mbuf.h"
#include "rte_errno.h"
#include "rte_hash.h"
#include "zqueue.h"

struct network_monitor_data {
    struct rte_hash *ht_table;
    struct zqueue *value_buffer; // preallocated buffer for stored value for the hash_table
    struct ipv4_5tuple temp_5tuple;
    long threshold;
    long counter;
    int core_id;

    int interaction_counter;
};

struct network_monitor_control {
    struct rte_hash *ht_table;
    struct zqueue *value_buffer; // preallocated buffer for stored value for the hash_table
    int core_id;

    int interaction_counter;
};


struct network_monitor_parameter {
    struct rte_hash *ht_table;
    struct zqueue *value_buffer; // preallocated buffer for stored value for the hash_table
    int core_id;
    int threshold;
    int interaction_counter;
};

struct network_monitor_data *network_monitor_init(struct network_monitor_parameter *nmp);


static inline void network_monitor_data_action(struct network_monitor_data *nmd, struct message *m) {
    int *value_lookup;

    nmd->counter = nmd->counter + 1;
//    if (nmd->counter == nmd->num_flow) {
//        nmd->temp_5tuple.proto = (uint8_t) nmd->core_id;
//        nmd->temp_5tuple.ip_dst = 0;
//        nmd->temp_5tuple.ip_src = 0;
//        nmd->temp_5tuple.port_src = 0;
//        nmd->temp_5tuple.port_dst = 0;
//        nmd->counter = 0;
//    } else {
//        ipv4_5tuple_next(&nmd->temp_5tuple);
//    }
//    DEBUG_PRINT("%d\n", nmd->counter);
    int ret = rte_hash_lookup_data((*nmd).ht_table, (void *) &nmd->temp_5tuple,
                                   (void **) &value_lookup);
    if (ret != -ENOENT) {
        *value_lookup += 1;
//        DEBUG_PRINT("core %d value:%d %d\n", nmd->core_id, *value_lookup, rte_hash_count((*nmd).ht_table));
//        m->event = FWD;// forward the packets
        return;
    } else {
        long *p_value = dequeue(nmd->value_buffer);
        *p_value = 1;
        int ret_add = rte_hash_add_key_data((*nmd).ht_table, &nmd->temp_5tuple, p_value);
        if (ret_add != 0) {
//            DEBUG_PRINT("error ");
        }
    }
    DEBUG_PRINT("core %d count %d\n", nmd->core_id, rte_hash_count((*nmd).ht_table));
//    m->event = FWD;// forward the packets
}

static inline void network_monitor_control_action(struct network_monitor_data *nmd, struct message *m) {
    int *value_lookup;

    //    DEBUG_PRINT("%d\n", nmd->counter);
    int ret = rte_hash_lookup_data((*nmd).ht_table, (void *) &(m->ipv45Tuple),
                                   (void **) &value_lookup);
    if (ret != -ENOENT) {
        *value_lookup += 1;
        //        DEBUG_PRINT("core %d value:%d %d\n", nmd->core_id, *value_lookup, rte_hash_count((*nmd).ht_table));
//        m->event = 1;// forward the packets
        return;
    } else {
        long *p_value = dequeue(nmd->value_buffer);
        if (p_value == NULL) {
//            m->event = 99; // drop the packet
        } else {
            *p_value = 1;
            int ret_add = rte_hash_add_key_data((*nmd).ht_table, &nmd->temp_5tuple, p_value);
            if (ret_add != 0) {
                //            DEBUG_PRINT("error ");
            }
        }
    }

    DEBUG_PRINT("core %d count %d\n", nmd->core_id, rte_hash_count((*nmd).ht_table));
//    m->event = 1;// forward the packets
}

static inline void decomposed_network_monitor_data_action(struct network_monitor_data *nmd, struct message *m) {
    long *value_lookup;

    nmd->counter = nmd->counter + 1;
    if (nmd->counter == (nmd->threshold-1)) {
        nmd->temp_5tuple.ip_src = 0;
        nmd->counter = 0;
    } else {
        ipv4_next(&nmd->temp_5tuple.ip_src);
    }

    int ret = rte_hash_lookup_data((*nmd).ht_table, (void *) &(nmd->temp_5tuple.ip_src),
                                   (void **) &value_lookup);
    if (ret != -ENOENT) { // old packet
        *value_lookup += 1; // update the counter
        if (*value_lookup % nmd->interaction_counter == 0) { // frequency
            m->ipv45Tuple = nmd->temp_5tuple;
//            m->event = TO_CONTROL_FWD; // forward the packet and send the message to the control actor
        } else { // dont have to tell the control actor for efficiency
//            m->event = FWD;// forward the packets
        }
        return;
    } else {
        long *p_value = dequeue(nmd->value_buffer);
        if (p_value == NULL) {
//            m->event = DROP_PACKET; // drop the packet
            return;
        }
        *p_value = 1;
        int ret_add = rte_hash_add_key_data((*nmd).ht_table, &(nmd->temp_5tuple.ip_src), p_value);
        if (ret_add != 0) {
                        DEBUG_PRINT("error ");
        }
        DEBUG_PRINT("core %d count %d\n", nmd->core_id, rte_hash_count((*nmd).ht_table));
//        m->event = FWD;
        return;
    }
}

static inline void decomposed_network_monitor_control_action(struct network_monitor_control *nmc, struct message *m) {

    long *value_lookup;
    int ret = rte_hash_lookup_data((*nmc).ht_table, (void *) &m->ipv45Tuple.ip_src,
                                   (void **) &value_lookup);
    if (ret != -ENOENT) { // old packet
        *value_lookup += nmc->interaction_counter; // update the counter
//        m->event = FREE_MESSAGE;
        return;
    } else {
        long *p_value = dequeue(nmc->value_buffer);
        if (p_value == NULL) {
//            m->event = FREE_MESSAGE; //  free the message
            return;
        }

        *p_value = nmc->interaction_counter;
        int ret_add = rte_hash_add_key_data((*nmc).ht_table, &m->ipv45Tuple.ip_src, p_value);
        if (ret_add != 0) {
            //            DEBUG_PRINT("error ");
        }
        DEBUG_PRINT("core %d count %d\n", nmc->core_id, rte_hash_count((*nmc).ht_table));
//        m->event = FREE_MESSAGE; //  free the message
        return;
    }
}

#endif //NF_PLATFORM_NM_H
