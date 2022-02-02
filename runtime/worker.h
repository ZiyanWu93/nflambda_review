//
// Created by anon on 8/30/2021.
//

#ifndef NF_PLATFORM_WORKER_H
#define NF_PLATFORM_WORKER_H

#include "common.h"
#include "rte_cycles.h"
#include <rte_malloc.h>
#include <rte_ethdev.h>
#include <rte_log.h>
#include <rte_timer.h>

enum worker_state {
    WORKER_IDEL,
    WORKER_READY,
    WORKER_RUNNING,
};


struct worker {
    bool suspend;
    enum worker_state *state;
    short core_id;
    int queue_id;
    int port_id;
    long rxn; // number of packets recieved
    long txn; // number of packets sent
    long rxs; // size of packets recieved
    long txs; // size of packet sent

    long operations_n;
    int max_pkt_burst;
    uint64_t start;
    volatile bool *dp_quit;
    struct mailbox self_mailbox; // used to talk to its higher-tier control actor
    struct rte_eth_dev_tx_buffer *txb;

    int first_action_id[MAXIMUM_PATH_INSTANCE]; // used when it is a data worker
    int first_action_num;

    int buckets[512]; // each entry maps to action_id // rss -> bucket_id -> action_id
    bool last_action;
    bool first_action;

    struct buffer *message_buffer;
    struct mailbox other_mailbox[48];

    struct action_table *actionTable;
    unsigned int starting_bucket;
    int bucket_per_shard;

    int *bucket_to_action_id;
};

static inline int send_the_unsent_packets(struct worker *p_nfunctor, unsigned portid) {
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

static inline transmit_packet(struct worker *p_nfunctor, struct message *m) {
//    DEBUG_PRINT("transmit packet");
    int nb_tx = rte_eth_tx_buffer(p_nfunctor->port_id, p_nfunctor->queue_id, p_nfunctor->txb,
                                  m->m);
    p_nfunctor->txs += rte_pktmbuf_pkt_len(m->m) * 8;
//    DEBUG_PRINT("%d, send packets!\n", p_nfunctor->core_id);
    m->path = -1;
}

static inline int event_handler(struct worker *p_worker, struct message *p_m) {
    struct message *inter_core_message;
    void (*select_action)(struct state *,
                          struct message *) = p_worker->actionTable->actionTableEntry[p_m->action_id].action;
    struct state *s = p_worker->actionTable->actionTableEntry[p_m->action_id].state;
    int core_id = p_worker->actionTable->actionTableEntry[p_m->action_id].core_id;
    if (core_id != p_worker->core_id) { // the message should be sent to other cores
        if (p_m->producer_id ==
            -1) { // if the message is created locally, then a new message should be created to be sent to other cores.
            inter_core_message = get_free_item(p_worker->message_buffer);
            if (inter_core_message == NULL) {

                rte_mbuf_raw_free(p_m->m);
                return 2; // no need to free the message
            }
            *inter_core_message = *p_m;
            inter_core_message->producer_id = p_worker->core_id;
            int ret = 11;
            while (ret != 0) {
                ret = rte_ring_enqueue(p_worker->other_mailbox[core_id].recv_ring, inter_core_message);
            }
            return 2;
        } else { // if the message is not from local, just pass the message
            int ret = -1;
            while (ret != 0) {
                ret = rte_ring_enqueue(p_worker->other_mailbox[core_id].recv_ring, p_m);
            }
            return 2;
        }

    }
//    DEBUG_PRINT("event_handler! %d \n",p_m->action_id);
    select_action(s, p_m);
    p_worker->operations_n += 1;
//    DEBUG_PRINT("event_handler! %d \n",p_m->path);
    if (p_m->path == -1) { // it is the result of transmit_packet
        return 1;
    } else {
        p_m->action_id = p_m->path;
        return 0;
    }
}

static inline void worker_action(struct worker *p_worker) {
    static int debug = 0;
    unsigned portid = 0, nb_rx, nb_tx;


    DEBUG_PRINT("worker entering %u\n", p_worker->core_id);

    int queue_id = p_worker->queue_id;
    struct rte_mbuf *pkts_burst[1024];

    struct message p_cm_local;
    struct message *p_cm;
    int ret;
    struct rte_mbuf *m;
    unsigned mask;
    int path_id;
    mask = (1 << 9) - 1;

    while (!*(p_worker->dp_quit)) {
        // only try to send the packets if it contains the last layer of action
        if (p_worker->last_action == true) {
            nb_tx = send_the_unsent_packets(p_worker, portid);
            if (nb_tx != 0) {
                p_worker->txn += nb_tx;
                //                p_worker->txs += nb_tx * rte_pktmbuf_pkt_len(m) * 8;
            }
        }

        ret = rte_ring_dequeue(p_worker->self_mailbox.recv_ring, (void **) &p_cm);
        while (ret == 0) {
            while (ret == 0) {
                ret = event_handler(p_worker, p_cm);
            }

            if (p_cm->producer_id == p_worker->core_id && ret == 1) {
                return_free_item(p_worker->message_buffer, p_cm);
            }

            ret = rte_ring_dequeue(p_worker->self_mailbox.recv_ring, (void **) &p_cm);
        }

        if (p_worker->first_action) {
            nb_rx = rte_eth_rx_burst(portid, queue_id,
                                     pkts_burst, 1024);

            p_worker->rxn += nb_rx;
            if (nb_rx != 0) {
                for (int j = 0; j < nb_rx; j++) {
                    p_cm_local.m = pkts_burst[j];
                    p_cm_local.producer_id = -1; // means the message is local
                    p_cm_local.core_id = p_worker->core_id;
                    p_cm_local.rss_hash = p_cm_local.m->hash.rss & mask;
                    p_cm_local.action_id = p_worker->buckets[p_cm_local.rss_hash];
                    p_worker->rxs += rte_pktmbuf_pkt_len(p_cm_local.m) * 8;
                    ret = 0;
                    while (ret == 0) {
                        ret = event_handler(p_worker, &p_cm_local);
                    }
                }
            }
        }
    }
}

#endif //NF_PLATFORM_WORKER_H
