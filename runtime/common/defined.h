//
// Created by anon on 5/3/21.
//

#ifndef NF_PLATFORM_DEFINED_H
#define NF_PLATFORM_DEFINED_H

#include "stdio.h"
#include "stdbool.h"
#include "stdint.h"
struct ipv4_5tuple {
    uint8_t proto;
    uint32_t ip_src;
    uint32_t ip_dst;
    uint16_t port_src;
    uint16_t port_dst;
};

//inline int fast_rand(void) {
//    g_seed = (214013*g_seed+2531011);
//    return (g_seed>>16)&0x7FFF;
//}


//static unsigned int g_seed;
//
//// Used to seed the generator.
//inline void fast_srand(int seed) {
//    g_seed = seed;
//}
//
//// Compute a pseudorandom integer.
//// Output value in rand_range [0, 32767]
//inline int fast_rand(void) {
//    g_seed = (214013*g_seed+2531011);
//    return (g_seed>>16)&0x7FFF;
//}

static inline void ipv4_5tuple_next(struct ipv4_5tuple *ipv45Tuple) {
    ipv45Tuple->port_src = ipv45Tuple->port_src + 1;
    if (ipv45Tuple->port_src == 0) {
        ipv45Tuple->port_dst += 1;
//        printf("a loop\n");
        if (ipv45Tuple->port_dst == 0) {
            ipv45Tuple->ip_src += 1;
//            printf("b loop\n");
            if (ipv45Tuple->ip_src == 0) {
                ipv45Tuple->ip_dst += 1;
            }
        }
    }
}

static inline void ipv4_next(uint32_t * ip) {
    *ip = *ip+1;
}



struct ipv4_5tuple_record{
    struct ipv4_5tuple Ipv4_tuple;
    int control_id;
    int data_id; // the sequence number not the lcore
};

struct server_ip_map {
    uint32_t bucket_id;
    uint32_t ip_dst_private;
};

struct Core_list
{
    unsigned int core_id[100];
    unsigned int core_number;
};

#define BURST_TX_DRAIN_US 5000 /* TX drain every ~200us */
#endif //NF_PLATFORM_DEFINED_H
