//
// Created by anon on 12/24/20.
//

#ifndef MULTICORE_FWDING_DPDK_CONFIG_H
#define MULTICORE_FWDING_DPDK_CONFIG_H

#include "common.h"
#include "nf/generic_mono.h"
#include "nf/mono_lb.h"
#include "port.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <rte_timer.h>
#include "rte_cycles.h"
#include "ddio.h"
#include "nf/fwd.h"
#include "nf/mq.h"
#include "nf/ss_lb.h"
#include "nf/generic_decomposed.h"
#include "nf/decomposed_lb.h"
#include "state/state.h"

#include "cat.h"
#include <bits/types/FILE.h>
#include <stdlib.h>
#include <sys/io.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <inttypes.h>
#include <unistd.h>
#include "yaml.h"
#include "common/set.h"
#include "nfvi.h"

#include "zgraph.h"
#include "worker.h"


#define NF_NUM 2
struct Config {
    int max_pkt_burst;
    int mbuf_cache_size;
    int num_mbufs;
    uint16_t nb_rxd;
    uint16_t nb_txd;
    struct port_parameter *portParameter;
    uint64_t hz;
    volatile bool dp_quit;
    volatile bool cp_quit;
    enum NF nf; //running nf
    int ring_size;

    struct physical_machine *pm;
    struct init_state_parameter initStateParameter;

    struct rte_eth_dev_info info;
    struct rte_eth_rss_reta_entry64 reta_conf[RSS_I_TABLE_SIZE / RTE_RETA_GROUP_SIZE]; // bucket_to_queue_id

    struct worker worker_list[48]; // 48 because there are 48 cores in our machines
    struct action_registration actionRegistration[200];
    struct state_registration stateRegistration[200];
    struct action_table actionTable;
    struct state_table stateTable;
    int queue_num;
    struct rte_hash *shared_state;

    uint64_t p_time;
    bool log_started;
    bool running;

    int bucket_to_action_id[512];
};

void config_init(char *config_file);


void install_flow();

//void destroy_nf();
void init_EAL(char *filename);

void output_log(struct Config *c);

#endif //MULTICORE_FWDING_DPDK_CONFIG_H