#ifndef NF_GENERIC_MONO_H
#define NF_GENERIC_MONO_H
#include "stdint.h"
#include "stdbool.h"
struct generic_mono
{
    int core_id;
    int queue_id;
    long rxn; // number of packets recieved
    long txn; // number of packets sent
    long rxs; // size of packets recieved
    long txs; // size of packet sent
    int max_pkt_burst;
    // struct rte_timer timer;
    uint64_t start;
    volatile bool *dp_quit;

    int frequency_control;
    int complexity_control;
    int complexity_data;
    int result;
    int control_interval;

    long control_packet;
    long data_packet;
    // int Eventset; // for PAPI
    // long long value[18];
    // bool flag;
    struct rte_eth_dev_tx_buffer *txb;

    struct rte_hash *data_state;
    long* data_value_buffer_d;
    int buffer_pointer_d;
    long data_state_size;
    long current_lookup;

    struct rte_hash *control_state;
    long control_state_size;
    long* data_value_buffer_c;
    int buffer_pointer_c;
};

struct generic_mono_parameters
{
    int core_id;
    int queue_id;
    int max_pkt_burst;
    volatile bool *dp_quit;

    int frequency_control;
    int complexity_control;
    int complexity_data;
    int control_interval;

    long control_state_size;
    long data_state_size;
};

int generic_mono_action(struct generic_mono *p_gm);
void generic_mono_create(struct generic_mono **p_gm);
void generic_mono_init(struct generic_mono *p_gm, struct generic_mono_parameters *p_p);
void generic_mono_destroy(struct generic_mono **p_gm);

#endif /* NF_GENERIC_MONO */
