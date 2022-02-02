//
// Created by anon on 4/24/21.
//

#include "generic_mono.h"
#include <rte_malloc.h>
#include <rte_ethdev.h>
#include <rte_log.h>
#include <rte_timer.h>
#include "../Config.h"
#include "../common/defined.h"


int generic_mono_action(struct generic_mono *p_gm) {
    // get 5-tuples
    // lookup using flow mapper hashing table
    // if miss, call control action
    // if hit, rewrite the destination ip and forward it

    uint32_t *DIP;
    struct rte_ether_hdr *rteEtherHdr;
    struct rte_ipv4_hdr *rteIpv4Hdr;
    struct rte_tcp_hdr *tcpHdr;
    struct ipv4_5tuple ipv45Tuple;
    struct rte_mbuf *pkts_burst[p_gm->max_pkt_burst * 2];
    int ret;
    unsigned lcore_id = rte_lcore_id();
    unsigned j, portid, nb_tx;
    unsigned nb_rx = 0;
    int *value_lookup;
    bool started = false;
    int queue_id = p_gm->queue_id;
    struct rte_mbuf *m;
    const uint64_t drain_tsc = (rte_get_tsc_hz() + US_PER_S - 1) / US_PER_S *
                               BURST_TX_DRAIN_US;

    struct rte_eth_dev_tx_buffer *txb = rte_zmalloc_socket("tx_buffer", RTE_ETH_TX_BUFFER_SIZE(p_gm->max_pkt_burst), 0,
                                                           1);
    uint64_t cur_tsc, diff_tsc, prev_tsc;
    prev_tsc = 0;

    if (txb == NULL)
        rte_exit(EXIT_FAILURE, "Cannot allocate buffer for tx on port %u\n",
                 portid);

    rte_eth_tx_buffer_init(txb, p_gm->max_pkt_burst);

    if (ret < 0)
        rte_exit(EXIT_FAILURE,
                 "Cannot set error callback for tx buffer on port %u\n",
                 portid);

    printf("generic_mono_action entering main loop on lcore %u; control_frequency %d\n", lcore_id,
           p_gm->frequency_control);

    p_gm->start = rte_rdtsc();
    int packet_size = 0;
    int result;

    p_gm->rxn = 0;
    p_gm->rxs = 0;
    p_gm->txn = 0;
    p_gm->txs = 0;

    int current = 0;
    bool control = false;

    while (!*(p_gm->dp_quit)) {
        cur_tsc = rte_rdtsc();
        diff_tsc = cur_tsc - prev_tsc;

        if (unlikely(diff_tsc > drain_tsc)) {
            nb_tx = rte_eth_tx_buffer_flush(portid, queue_id, txb);
            p_gm->txn += nb_tx;
            p_gm->txs += nb_tx * packet_size * 8;
            prev_tsc = cur_tsc;
        }
        nb_rx = rte_eth_rx_burst(portid, queue_id, pkts_burst, p_gm->max_pkt_burst);
        p_gm->rxn += nb_rx;
        if (unlikely(nb_rx == 0))
            continue;

        if (nb_rx != 0) {
            for (j = 0; j < nb_rx; j++) {
                m = pkts_burst[j];
                packet_size = rte_pktmbuf_pkt_len(m);
                p_gm->rxs += packet_size * 8;

                if (p_gm->frequency_control > 0 && current < p_gm->frequency_control) {
                    control = false;
                } else if (p_gm->frequency_control < 0 && current == p_gm->frequency_control) {
                    control = false;
                } else if (p_gm->frequency_control != 0) {
                    control = true;
                } else {
                    control = false;
                }

                if (control == true) {

                    ipv4_5tuple_next(&ipv45Tuple);
                    result = rte_hash_lookup_data((*p_gm).control_state, (void *) &ipv45Tuple,
                                                  (void **) &value_lookup);

                    if (result == -ENOENT) {
                        int ret_add = rte_hash_add_key_data((*p_gm).control_state, &ipv45Tuple,
                                                            &(*p_gm).data_value_buffer_c[(*p_gm).buffer_pointer_c]);
                        (*p_gm).buffer_pointer_c = ((*p_gm).buffer_pointer_c + 1) % 100;;
                    } else {
                    }
                    operation(p_gm->complexity_control);
                    if (p_gm->frequency_control < 0) {
                        current -= 1;
                    } else {
                        current = 0;
                    }
                } else {
                    p_gm->data_packet += packet_size * 8;
                    if (p_gm->frequency_control > 0)
                        current += 1;
                    else if (p_gm->frequency_control < 0) {
                        current = 0;
                    }
                }
                rteEtherHdr = rte_pktmbuf_mtod(m,
                struct rte_ether_hdr *);
                if (rteEtherHdr->ether_type == rte_be_to_cpu_16(RTE_ETHER_TYPE_IPV4)) {
                    rteIpv4Hdr = rte_pktmbuf_mtod_offset(m,
                    struct rte_ipv4_hdr *,
                    sizeof(struct rte_ether_hdr));
                    if (rteIpv4Hdr->next_proto_id == IPPROTO_TCP) {
                        tcpHdr = rte_pktmbuf_mtod_offset(m,
                        struct rte_tcp_hdr *,
                        sizeof(struct rte_ether_hdr) +
                        sizeof(struct rte_ipv4_hdr));
                    }
                }
                ipv4_5tuple_next(&ipv45Tuple);
                result = rte_hash_lookup_data((*p_gm).data_state, (void *) &ipv45Tuple,
                                              (void **) &value_lookup);

                if (result == -ENOENT) {
                    int ret_add = rte_hash_add_key_data((*p_gm).data_state, &ipv45Tuple,
                                                        &(*p_gm).data_value_buffer_d[(*p_gm).buffer_pointer_d]);
                    (*p_gm).buffer_pointer_d = ((*p_gm).buffer_pointer_d + 1) % 100;;
                } else {
                }

                operation(p_gm->complexity_data);
                nb_tx = rte_eth_tx_buffer(portid, queue_id, txb, m);
                p_gm->txs += nb_tx * rte_pktmbuf_pkt_len(m) * 8;
                p_gm->txn += nb_tx;
            }
        }
    }
    nb_tx = rte_eth_tx_buffer_flush(portid, queue_id, txb);
    rte_free(txb);
    rte_hash_free(p_gm->control_state);
    rte_hash_free(p_gm->data_state);
    free((*p_gm).data_value_buffer_d);
    free((*p_gm).data_value_buffer_c);
    printf("generic_mono_action entering main loop on lcore done %u\n", lcore_id);
}

void generic_mono_create(struct generic_mono **p_gm) {
    *p_gm = malloc(sizeof(struct generic_mono));
}

void generic_mono_init(struct generic_mono *p_gm, struct generic_mono_parameters *p_p) {
    DEBUG_PRINT("generic_mono_init started\n");
    // *p_gm = malloc(sizeof(struct generic_mono));
    (*p_gm).rxn = 0;
    (*p_gm).txn = 0;
    (*p_gm).rxs = 0;
    (*p_gm).txs = 0;
    (*p_gm).data_packet = 0;
    (*p_gm).dp_quit = p_p->dp_quit;
    (*p_gm).max_pkt_burst = p_p->max_pkt_burst;
    (*p_gm).queue_id = p_p->queue_id;
    (*p_gm).complexity_data = p_p->complexity_data;
    (*p_gm).complexity_control = p_p->complexity_control;
    (*p_gm).frequency_control = p_p->frequency_control;
    (*p_gm).core_id = p_p->core_id;
    (*p_gm).data_state_size = p_p->data_state_size;
    (*p_gm).control_state_size = p_p->control_state_size;

//    printf("%ld\n", p_p->data_state_size);
//    printf("%ld\n", p_p->control_state_size);
    // initialization of data state
    char s[64];
    snprintf(s, sizeof(s), "gm data state %d", (*p_gm).core_id);
    int data_state_size = 2000;
    if (p_gm->data_state_size != 0) {
        data_state_size = p_gm->data_state_size;
        printf("%ld\n", data_state_size);
    } else {
        printf("default data state size\n");
        exit(0);
    };

    struct rte_hash_parameters ht_table_type_fm = {
            .name = s,
            .entries = data_state_size, // arbitrary
            .key_len = sizeof(struct ipv4_5tuple),
            .hash_func = rte_jhash,
            .hash_func_init_val = 0,
            .socket_id = 1};
    (*p_gm).data_value_buffer_d = malloc(sizeof(long) * 100);
    for (int i = 0; i < 100; i++) {
        (*p_gm).data_value_buffer_d[i] = (long) i;
    }

    (*p_gm).data_state_size = data_state_size;
    (*p_gm).data_state = rte_hash_create(&ht_table_type_fm);
    if ((*p_gm).data_state == NULL) {
        printf("Unable to create the generic monolithic data state on socket %d\n", 1);
        exit(0);
    }

    // initialization of control state
    snprintf(s, sizeof(s), "gm control state %d", (*p_gm).core_id);
    int control_state_size = 2000;
    if (p_gm->control_state_size != 0) {
        control_state_size = p_gm->control_state_size;
        printf("%ld\n", control_state_size);
    } else {
        printf("default control state size\n");
        exit(0);
    };

    ht_table_type_fm.entries = control_state_size;

    (*p_gm).data_value_buffer_c = malloc(sizeof(long) * 100);
    for (int i = 0; i < 100; i++) {
        (*p_gm).data_value_buffer_c[i] = (long) i;
    }
    (*p_gm).control_state_size = control_state_size;
    (*p_gm).control_state = rte_hash_create(&ht_table_type_fm);
    if ((*p_gm).control_state == NULL) {
        printf("Unable to create generic_decomposed_control_init on socket %d\n", 1);
        exit(0);
    }

    DEBUG_PRINT("generic_mono_init done\n");
}

void generic_mono_destroy(struct generic_mono **p_gm) {
    // free the hash_table
    // rte_free((**p_gm).txb);
    free(*p_gm);
    return;
}