//
// Created by anon on 8/30/2021.
//

#ifndef NF_PLATFORM_NFUNCTOR_H
#define NF_PLATFORM_NFUNCTOR_H

#include "common.h"
#include "rte_cycles.h"
#include <rte_malloc.h>
#include <rte_ethdev.h>
#include <rte_log.h>
#include <rte_timer.h>

struct nfunctor {
    bool suspend;
    int id;
    short core_id;
    int queue_id;
    int port_id;
    long rxn; // number of packets recieved
    long txn; // number of packets sent
    long rxs; // size of packets recieved
    long txs; // size of packet sent
    int pkt_len;
    int max_pkt_burst;
    uint64_t start;
    volatile bool *dp_quit;
    struct mailbox self_mailbox; // used to talk to its higher-tier control actor
    struct rte_eth_dev_tx_buffer *txb;

    int first_action_id[MAXIMUM_PATH_INSTANCE]; // used when it is a data nfunctor
    int first_action_num;

    bool last_action;
    struct buffer *message_buffer;
    struct mailbox other_mailbox[48];

    struct action_table *actionTable;
    unsigned int starting_bucket;
    int bucket_per_shard;

    int* bucket_to_action_id;
};

static inline int send_the_unsent_packets(struct nfunctor *p_nfunctor, unsigned portid) {
    static uint64_t cur_tsc = 0, prev_tsc = 0;
    const uint64_t drain_tsc = (rte_get_tsc_hz() + US_PER_S - 1) / US_PER_S *
                               BURST_TX_DRAIN_US;
    cur_tsc = rte_rdtsc();
    uint64_t diff_tsc = cur_tsc - prev_tsc;
    if (unlikely(diff_tsc > drain_tsc)) {
        int nb_tx = rte_eth_tx_buffer_flush(portid, p_nfunctor->queue_id, p_nfunctor->txb);
        prev_tsc = cur_tsc;
        return nb_tx;
    }
    return 0;
}

static inline transmit_packet(struct nfunctor *p_nfunctor, struct message *m) {
    int nb_tx = rte_eth_tx_buffer(p_nfunctor->port_id, p_nfunctor->queue_id, p_nfunctor->txb,
                                  m->m);
    p_nfunctor->txn += nb_tx;
    p_nfunctor->txs += rte_pktmbuf_pkt_len(m->m) * 8;
    m->path = 0;
}

static inline int event_handler(struct nfunctor *p_nfunctor, struct message *p_m) {
    void (*select_action)(struct state *,
                          struct message *) = p_nfunctor->actionTable->actionTableEntry[p_m->action_id].action;
    struct state *s = p_nfunctor->actionTable->actionTableEntry[p_m->action_id].state;
    int core_id = p_nfunctor->actionTable->actionTableEntry[p_m->action_id].core_id;
    if (core_id != p_nfunctor->core_id) {
        DEBUG_PRINT("unimplemented\n");
    }
    select_action(s, p_m);
    if (p_m->path == -1) {
        if(p_nfunctor->actionTable->actionTableEntry[p_m->action_id].path_instance_number[MAXIMUM_PATH-1]==0){
            transmit_packet(p_nfunctor, p_m);
            return 1;
        }
        else{
            p_m->action_id = p_nfunctor->actionTable->actionTableEntry[p_m->action_id].path[MAXIMUM_PATH - 1][0];
            return 0;
        }

    } else {
        p_m->action_id = p_nfunctor->actionTable->actionTableEntry[p_m->action_id].path[p_m->path][0];
        return 0;
    }
}

static inline void nfunctor_action(struct nfunctor *p_nfunctor) {
    static int debug = 0;
    unsigned portid = 0, nb_rx, nb_tx;


    DEBUG_PRINT("Nfunctor entering %u\n", p_nfunctor->core_id);

    int queue_id = p_nfunctor->queue_id;
    struct rte_mbuf *pkts_burst[1024];

    struct message p_cm_local;
    struct message *p_cm;
    int ret;
    struct rte_mbuf *m;
    unsigned mask;
    int path_id;
    mask = (1 << 9) - 1;

//    int starting_bucket = 0;

    while (!*(p_nfunctor->dp_quit)) {

        // only try to send the packets if it contains the last action
        if (p_nfunctor->last_action == true) {
            nb_tx = send_the_unsent_packets(p_nfunctor, portid);
            if (nb_tx != 0) {
                p_nfunctor->txn += nb_tx;
                //                p_nfunctor->txs += nb_tx * rte_pktmbuf_pkt_len(m) * 8;
            }
        }

        ret = rte_ring_dequeue(p_nfunctor->self_mailbox.recv_ring, (void **) &p_cm);
        while (ret == 0) {
            ret = 0;
            while (ret == 0) {
                ret = event_handler(p_nfunctor, p_cm);
            }
            ret = rte_ring_dequeue(p_nfunctor->self_mailbox.recv_ring, (void **) &p_cm);
        }

        // recieves the packet, if the first action id is -1, no need to recieve packets
        if (p_nfunctor->first_action_num != 0) {
            nb_rx = rte_eth_rx_burst(portid, queue_id,
                                     pkts_burst, 1024);

            p_nfunctor->rxn += nb_rx;
            if (nb_rx != 0) {
                for (int j = 0; j < nb_rx; j++) {
                    p_cm_local.m = pkts_burst[j];
                    p_cm_local.core_id = p_nfunctor->core_id;
                    p_cm_local.action_id = p_nfunctor->first_action_id;
                    p_cm_local.rss_hash = p_cm_local.m->hash.rss & mask;
                    p_cm_local.action_id = p_nfunctor->bucket_to_action_id[p_cm_local.rss_hash];
//                    DEBUG_PRINT("nfunctor_id is %d starting bucket is %d, hash is %d\n", p_nfunctor->core_id,
//                                p_nfunctor->starting_bucket, p_cm_local.rss_hash);
//                    DEBUG_PRINT("path is %d\n", path_id);
                    p_nfunctor->rxs += rte_pktmbuf_pkt_len(p_cm_local.m) * 8;
                    ret = 0;
                    while (ret == 0) {
                        ret = event_handler(p_nfunctor, &p_cm_local);
                    }
                }
            }
        }
    }
}

#endif //NF_PLATFORM_NFUNCTOR_H
