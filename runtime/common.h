//
// Created by anon on 8/30/2021.
//

#ifndef NF_PLATFORM_COMMON_H
#define NF_PLATFORM_COMMON_H

#include "zqueue.h"
#include "zbuffer.h"
#include "common/defined.h"


#define BURST_TX_DRAIN_US 100
#define LOG_INTERVAL_US 1000000
#define SYSTEM_NF_OFFSET 100
#define RSS_I_TABLE_SIZE 512
enum NF {
    NF_FWD = 0,
};

struct message {
    int core_id;
    int producer_id;
    int action_id; // used by the worker to find the action
    int path; // modify by the action
    union {
        struct rte_mbuf *m;
        struct ipv4_5tuple ipv45Tuple;
        char *data;
    };
    uint32_t rss_hash;
};

enum Config_Item {
    max_pkt_burst,
    port,
};

#define MAXIMUM_PATH 10
#define MAXIMUM_PATH_INSTANCE 24


struct action_table_entry {
    void (*action)(struct state *, struct message *);
    int path[MAXIMUM_PATH][MAXIMUM_PATH_INSTANCE];
    int path_instance_number[MAXIMUM_PATH];
    struct state* state;
    int core_id;
};

struct action_table {
    struct action_table_entry actionTableEntry[200];
    int num;
};

struct state_table_entry {
    struct state *state;
    int state_registration_id;
};

struct state_table {
    struct state_table_entry stateTableEntry[200];
    int num;
};

struct init_state_parameter {
    int execution_id;
    int control_frequency;
    int data_complexity;
    int control_complexity;
    int state_size;
    int rand_range; // the rand_range of random number
    int data_state_size;
    int control_state_size;
    int path[MAXIMUM_PATH][MAXIMUM_PATH_INSTANCE];
    int path_instance_number[MAXIMUM_PATH];
    struct rte_hash* shared_state;
};

static inline int __attribute__((optimize("O0"))) operation(int complexity) {
    int i;
    double tmp = 1.1;

    for (i = 0; i < complexity; i++) {
        tmp = tmp + 100;
    }
    return tmp;
}

struct mailbox {
    struct rte_ring *recv_ring;
};


struct switch_table_item {
    enum NF nf;

    //    unsigned short event;
    void (*action)(void *, struct message *);

    void *state;
    unsigned core_id;
    char *action_name;
};

#define SWITCH_TABLE_SIZE 2 * SYSTEM_NF_OFFSET
#define SWITCH_TABLE_NF_NUM 50
// action_id(NF) + event = (method, state)
struct switch_table {
    int user_nf;
    int system_nf;
    int user_action;
    int system_action;
    struct switch_table_item switchTableItem[SWITCH_TABLE_SIZE];
    short core_id;
    int offset[SWITCH_TABLE_SIZE];
};


int chain_action(struct switch_table *switchTable, int action_id_1, int action_id_2);

static inline void dump_switch_table(struct switch_table *switchTable) {
//    printf("user nf number: %d\n", switchTable->user_nf);
    for (int i = 0; i < switchTable->user_action; i++) {
        printf("%d nf %d %s %d\n", i, switchTable->switchTableItem[i].nf, switchTable->switchTableItem[i].action_name,
               switchTable->offset[i]);
    }

    for (int i = 0; i < switchTable->system_action; i++) {
        printf("%d system %d %s %d\n", SYSTEM_NF_OFFSET + i, switchTable->switchTableItem[SYSTEM_NF_OFFSET + i].nf,
               switchTable->switchTableItem[SYSTEM_NF_OFFSET + i].action_name,
               switchTable->offset[SYSTEM_NF_OFFSET + i]);
    }
}

static inline void dump_switch_table_str(struct switch_table *switchTable, char *str) {
    //    printf("user nf number: %d\n", switchTable->user_nf);
    int offset = 0;
    for (int i = 0; i < switchTable->user_action; i++) {
        offset += sprintf(&str[offset], "%d nf %d %s %d %d\n", i, switchTable->switchTableItem[i].nf,
                          switchTable->switchTableItem[i].action_name,
                          switchTable->offset[i], switchTable->switchTableItem[i].core_id);
    }

    for (int i = 0; i < switchTable->system_action; i++) {
        offset += sprintf(&str[offset], "%d system %d %s %d %d\n", SYSTEM_NF_OFFSET + i,
                          switchTable->switchTableItem[SYSTEM_NF_OFFSET + i].nf,
                          switchTable->switchTableItem[SYSTEM_NF_OFFSET + i].action_name,
                          switchTable->offset[SYSTEM_NF_OFFSET + i],
                          switchTable->switchTableItem[SYSTEM_NF_OFFSET + i].core_id);
    }
}


#define REGISTRATION_SIZE 10

// network function is the basic unit of registration
// because we only focus on stateful network function here
// all stateful network function can be decompmosed into data and control components
// state can be shared, partitionable, replicable
// state that cannot be partitioned: based on the overall state of the system. e.g. network monitor
// state that can be partitioned: resource-class. NAT-based
// state that can be replicated: firewall(DPI)
struct registeration {
    char *network_function_name;

    void (*action[REGISTRATION_SIZE])(void *, struct message *);

    void *(*init[REGISTRATION_SIZE])(void *);

    char action_name[REGISTRATION_SIZE][100];
    int action_number;
    struct init_state_parameter initStateParameter;
};


struct mailbox create_mailbox(int i, int ring_size);



struct action_registration {
    void (*action)(struct state *, struct message *);

//    void *(*init)(struct init_state_parameter*);
    char action_name[100];

};

struct state_registration {
    void *(*init)(struct init_state_parameter *);
    char state_name[100];
};



void register_action(struct action_registration *actionRegistration, void (*action)(void *, struct message *),
                     char *action_name);

//void register_state(struct state_registration *stateRegistration, void *(*init)(void *), char *state_name);

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, \
__FILE__, __LINE__, __func__, ##args)
#else
#define DEBUG_PRINT(...) do{ } while ( false )
#endif


#endif //NF_PLATFORM_COMMON_H
