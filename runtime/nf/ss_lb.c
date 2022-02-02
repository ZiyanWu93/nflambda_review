//
// Created by anon on 5/3/21.
//

#include "ss_lb.h"

static struct server_ip_map server_ip_map_array[] = {
        // to note: we can use public dst_ip as the flow id
        // (index id, ip_dst_private)
        {0, RTE_IPV4(1, 0, 0, 0)},
        {1, RTE_IPV4(2, 0, 0, 0)},
        {2, RTE_IPV4(3, 0, 0, 0)},
        {3, RTE_IPV4(4, 0, 0, 0)},
        {4, RTE_IPV4(5, 0, 0, 0)},
        {5, RTE_IPV4(6, 0, 0, 0)},
        {6, RTE_IPV4(7, 0, 0, 0)},
        {7, RTE_IPV4(8, 0, 0, 0)},
        {8, RTE_IPV4(9, 0, 0, 0)},
        {9, RTE_IPV4(10, 0, 0, 0)},
};

struct ss_lb_message{
    unsigned lcore;
    struct ipv4_5tuple * ipv45Tuple;
};

int lb_data_action(struct lb_data *lb_data) {

//     get 5-tuples
//     lookup using flow mapper hashing table
//     if miss, call control action
//     if hit, rewrite the destination ip and forward it

    void *msg;
    struct ss_lb_message sslbmessage;
    uint32_t *DIP;
    struct rte_ether_hdr *rteEtherHdr;
    struct rte_ipv4_hdr *rteIpv4Hdr;
    struct rte_tcp_hdr *tcpHdr;
    struct rte_udp_hdr *udpHdr;
    struct ipv4_5tuple ipv45Tuple;
    struct rte_mbuf *pkts_burst[lb_data->max_pkt_burst * 2];
    int ret;
    unsigned lcore_id = lb_data->core_id;
    sslbmessage.lcore = lcore_id;
    unsigned j, portid, nb_tx;
    unsigned nb_rx = 0;
    bool started = false;
    int queue_id = lb_data->queue_id;
    struct rte_mbuf *m;
    const uint64_t drain_tsc = (rte_get_tsc_hz() + US_PER_S - 1) / US_PER_S *
                               BURST_TX_DRAIN_US;

    uint64_t cur_tsc, diff_tsc, prev_tsc;
    prev_tsc = 0;

    struct rte_eth_dev_tx_buffer *txb = rte_zmalloc_socket("tx_buffer",
                                                           RTE_ETH_TX_BUFFER_SIZE(lb_data->max_pkt_burst) * 2, 0,
                                                           1);
    if (txb == NULL)
        rte_exit(EXIT_FAILURE, "Cannot allocate buffer for tx on port %u\n",
                 portid);

    rte_eth_tx_buffer_init(txb, lb_data->max_pkt_burst);

    if (ret < 0)
        rte_exit(EXIT_FAILURE,
                 "Cannot set error callback for tx buffer on port %u\n",
                 portid);

    printf("LB data entering main loop on lcore %u\n", lcore_id);

    lb_data->rxn = 0;
    lb_data->txn = 0;
    lb_data->last_port = 0;
    int packet_size = 0;
    int result;

    while (!*(lb_data->dp_quit)) {
        cur_tsc = rte_rdtsc();
        diff_tsc = cur_tsc - prev_tsc;

        if (unlikely(diff_tsc > drain_tsc)) {
            nb_tx = rte_eth_tx_buffer_flush(portid, queue_id, txb);
            lb_data->txn += nb_tx;
            lb_data->txs += nb_tx * packet_size * 8;
            prev_tsc = cur_tsc;
        }
        nb_rx = rte_eth_rx_burst(portid, queue_id, pkts_burst, lb_data->max_pkt_burst);
        lb_data->rxn += nb_rx;
        if (unlikely(nb_rx == 0))
            continue;

        if (nb_rx != 0) {
            if (unlikely(started == false))
                lb_data->start = rte_rdtsc();

            started = true;
            // use rteIpv4Hdr as pointer will lead to segmentation fault // solution: the checking of different headers seems to be necessary
            for (j = 0; j < nb_rx; j++) {
                //                rte_prefetch0(rte_pktmbuf_mtod(m,void *)); this will lead to segmentation fault?
                m = pkts_burst[j];
                packet_size = rte_pktmbuf_pkt_len(m);
                lb_data->rxs += packet_size * 8;
                rteEtherHdr = rte_pktmbuf_mtod(m, struct rte_ether_hdr *);

                if (rteEtherHdr->ether_type == rte_be_to_cpu_16(RTE_ETHER_TYPE_IPV4)) {
                    rteIpv4Hdr = rte_pktmbuf_mtod_offset(m, struct rte_ipv4_hdr *,
                                                         sizeof(struct rte_ether_hdr));
                    ipv45Tuple.ip_src = rte_be_to_cpu_32((rteIpv4Hdr->src_addr));
                    ipv45Tuple.ip_dst = rte_be_to_cpu_32((rteIpv4Hdr->dst_addr));
                    ipv45Tuple.proto = (uint8_t) rteIpv4Hdr->next_proto_id;
                    if (rteIpv4Hdr->next_proto_id == IPPROTO_TCP) {
                        tcpHdr = rte_pktmbuf_mtod_offset(m, struct rte_tcp_hdr *,
                                                         sizeof(struct rte_ether_hdr) +
                                                         sizeof(struct rte_ipv4_hdr));
                        ipv45Tuple.port_src = be16toh(tcpHdr->src_port);
                        ipv45Tuple.port_dst = be16toh(tcpHdr->dst_port);
                    } else if  (rteIpv4Hdr->next_proto_id == IPPROTO_UDP){
                        udpHdr = rte_pktmbuf_mtod_offset(m, struct rte_udp_hdr *,
                                                         sizeof(struct rte_ether_hdr) +
                                                         sizeof(struct rte_ipv4_hdr));
                        ipv45Tuple.port_src = be16toh(udpHdr->src_port);
                        ipv45Tuple.port_dst = be16toh(udpHdr->dst_port);
                    }

                    result = rte_hash_lookup_data(lb_data->ht_table_fm, (void *) &ipv45Tuple,
                                                  (void **) &DIP);
                    if (result == -ENOENT) { // not found
                        lb_data->unfound += 1;
                        // send message to control actor
                        sslbmessage.ipv45Tuple = &ipv45Tuple;
                        // printf("enqueue\n");
                        rte_ring_enqueue(lb_data->send_ring, &sslbmessage);
                        while (rte_ring_dequeue(lb_data->recv_ring, &msg) < 0) {
                            if(*(lb_data->dp_quit))
                                return;
                        }
//                            printf("core %u: Received '%s'\n", lcore_id, (uint32_t *)msg);
                        // recv message to control actor
                        int ret_add = rte_hash_add_key_data(lb_data->ht_table_fm, &ipv45Tuple,
                                                            msg);
                        switch (ret_add) {
                            case -ENOSPC:
                                lb_data->key_full += 1;
                                break;
                            case 0:
                                break;
                            case -EINVAL:
                                break;
                        }
                        rteIpv4Hdr->dst_addr = *(rte_be32_t *) msg;
                    } else {
                        rteIpv4Hdr->dst_addr = *DIP;
                        lb_data->found += 1;
                    }
                }
                nb_tx = rte_eth_tx_buffer(portid, queue_id, txb, m);
                lb_data->txs += nb_tx * rte_pktmbuf_pkt_len(m) * 8;
                lb_data->txn += nb_tx;
            }
        }
    }
}

void lb_data_init(struct lb_data **p_lb_data, struct lb_data_parameters lbDataParameters) {
    printf("lb_data_init started\n");
    rte_timer_subsystem_init(); // necessary to put it here?
    if (*p_lb_data == NULL) {
        *p_lb_data = malloc(sizeof(struct lb_data));
        rte_timer_init(&((**p_lb_data).timer));
        (**p_lb_data).rxn = 0;
        (**p_lb_data).txn = 0;
        (**p_lb_data).rxs = 0;
        (**p_lb_data).txs = 0;
        (**p_lb_data).key_full = 0;
        (**p_lb_data).dp_quit = lbDataParameters.dp_quit;
        (**p_lb_data).max_pkt_burst = lbDataParameters.max_pkt_burst;
        (**p_lb_data).queue_id = lbDataParameters.queue_id;
        (**p_lb_data).core_id = lbDataParameters.lcore_id;
        (**p_lb_data).found = 0;
        (**p_lb_data).unfound = 0;
        (**p_lb_data).recv_ring = lbDataParameters.recv_ring;
        (**p_lb_data).send_ring = lbDataParameters.send_ring;
        char s[64];
        /* create hash */
        snprintf(s, sizeof(s), "hash %d", lbDataParameters.queue_id);
        struct rte_hash_parameters ht_table_type_fm = {
                .name = s,
                .entries = lbDataParameters.num_entry, // arbitrary
                .key_len = sizeof(struct ss_lb_message),
                .hash_func = rte_jhash,
                .hash_func_init_val = 0,
                .socket_id = 1};
        (**p_lb_data).ht_table_fm = rte_hash_create(&ht_table_type_fm);
        if ((**p_lb_data).ht_table_fm == NULL) {
            printf("Unable to create the monolb_fm table on socket %d\n", 1);
        }
    } else {
        printf("failed!");
    }
    printf("lb_data_init ended\n");
}

void lb_control_ss_action(struct lb_control_ss *lbControlSs) {
    void *msg;
    int i=0;
    lbControlSs->running = true;
    unsigned lcore_id = rte_lcore_id();
    printf("LB control entering main loop on lcore %u\n", lcore_id);
    while (!*(lbControlSs->dp_quit)) {
        for (i=0; i< lbControlSs->n_data_component;i++){
            if(rte_ring_dequeue(lbControlSs->recv_ring_plist[i], &msg)<0){
                continue;
            }
            else{
                // printf("receive from %u\n", ((struct ss_lb_message*) msg)->lcore);
                lbControlSs->control_count += 1;
                rte_ring_enqueue(lbControlSs->send_ring_plist[i], (void *) &server_ip_map_array[0].ip_dst_private);
            }
        }

//        printf("core %u: Received '%s'\n", lcore_id, (char *)msg);
    }
}

void lb_control_ss_init(struct lb_control_ss **p_lb_control, int n_data_component, int max_pkt_burst,
                        volatile bool *dp_quit) {
    printf("lb_control_ss_init start\n");
    char s[64];
    unsigned flags = 0;
    unsigned ring_size = 64;
    struct lb_data_parameters lbDataParameters;
    // how many data componenets you have
    *p_lb_control = malloc(sizeof(struct lb_control_ss));
    (**p_lb_control).recv_ring_plist = malloc(sizeof(struct rte_ring *) * 24);
    (**p_lb_control).send_ring_plist = malloc(sizeof(struct rte_ring *) * 24);
    (**p_lb_control).lbData = malloc(sizeof(struct lb_data *) * 24);

    (**p_lb_control).n_data_component = n_data_component;
    (**p_lb_control).running = false;
    (**p_lb_control).dp_quit = dp_quit;
    (**p_lb_control).control_count = 0;

    // create "connections" for each data componenet  
    for (int i = 0; i < n_data_component; i++) {
        snprintf(s, sizeof(s), "lb_control recv %d", i);
        (**p_lb_control).recv_ring_plist[i] = rte_ring_create(s, ring_size, rte_socket_id(), flags);
        snprintf(s, sizeof(s), "lb_control send %d", i);
        (**p_lb_control).send_ring_plist[i] = rte_ring_create(s, ring_size, rte_socket_id(), flags);
    }
    printf("lb_control_ss_init done\n");
}

