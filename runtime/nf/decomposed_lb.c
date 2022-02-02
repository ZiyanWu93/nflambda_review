//
// Created by anon on 4/26/21.
//

#include "decomposed_lb.h"

#include "../common/defined.h"

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

static inline int __attribute__((optimize("O0"))) operation(int complexity)
{
    int i;
    double tmp = 1.1;

    for (i = 1; i < complexity; i++)
    {
        tmp = tmp + 100;
    }
    return tmp;
}

int decomposed_LB_data_action(struct decomposed_LB_data *dlb_d)
{
    void *msg;
    uint32_t *DIP;
    struct rte_ether_hdr *rteEtherHdr;
    struct rte_ipv4_hdr *rteIpv4Hdr;
    struct rte_tcp_hdr *tcpHdr;
    struct rte_udp_hdr *udpHdr;
    struct ipv4_5tuple ipv45Tuple;

    struct rte_mbuf *pkts_burst[dlb_d->max_pkt_burst * 2];
    int ret;
    unsigned lcore_id = rte_lcore_id();
    unsigned j, portid = 0, nb_tx;
    unsigned nb_rx = 0;
    int ret_add = 0;
    bool started = false;
    int queue_id = dlb_d->queue_id;
    struct rte_mbuf *m;
    const uint64_t drain_tsc = (rte_get_tsc_hz() + US_PER_S - 1) / US_PER_S *
                               BURST_TX_DRAIN_US;

    uint64_t cur_tsc, diff_tsc, prev_tsc = 0;
    int result = 0, packet_size = 0;

    struct rte_eth_dev_tx_buffer *txb = rte_zmalloc_socket("tx_buffer",
                                                           RTE_ETH_TX_BUFFER_SIZE(dlb_d->max_pkt_burst), 0,
                                                           1);
    if (txb == NULL)
        rte_exit(EXIT_FAILURE, "Cannot allocate buffer for tx on port %u\n",
                 portid);

    rte_eth_tx_buffer_init(txb, dlb_d->max_pkt_burst);

    if (ret < 0)
        rte_exit(EXIT_FAILURE,
                 "Cannot set error callback for tx buffer on port %u\n",
                 portid);

    printf("decomposed_LB_data_action entering main loop on lcore %u\n", lcore_id);

    dlb_d->start = rte_rdtsc();

    while (!*(dlb_d->dp_quit))
    {
        cur_tsc = rte_rdtsc();
        diff_tsc = cur_tsc - prev_tsc;

        if (unlikely(diff_tsc > drain_tsc))
        {
            nb_tx = rte_eth_tx_buffer_flush(portid, queue_id, txb);
            dlb_d->txn += nb_tx;
            dlb_d->txs += nb_tx * packet_size * 8;
            prev_tsc = cur_tsc;
        }
        // DEBUG_PRINT("pkts_burst %d dlb_d->max_pkt_burst %d\n", pkts_burst, dlb_d->max_pkt_burst);
        nb_rx = rte_eth_rx_burst(portid, queue_id, pkts_burst, dlb_d->max_pkt_burst);
        dlb_d->rxn += nb_rx;
        if (unlikely(nb_rx == 0))
            continue;

        if (nb_rx != 0)
        {
            for (j = 0; j < nb_rx; j++)
            {

                m = pkts_burst[j];
                packet_size = rte_pktmbuf_pkt_len(m);
                dlb_d->rxs += packet_size * 8;
                rteEtherHdr = rte_pktmbuf_mtod(m, struct rte_ether_hdr *);
                if (rteEtherHdr->ether_type == rte_be_to_cpu_16(RTE_ETHER_TYPE_IPV4))
                {
                    rteIpv4Hdr = rte_pktmbuf_mtod_offset(m, struct rte_ipv4_hdr *,
                                                         sizeof(struct rte_ether_hdr));
                    ipv45Tuple.ip_src = rte_be_to_cpu_32((rteIpv4Hdr->src_addr));
                    ipv45Tuple.ip_dst = rte_be_to_cpu_32((rteIpv4Hdr->dst_addr));
                    ipv45Tuple.proto = (uint8_t)rteIpv4Hdr->next_proto_id;
                    if (rteIpv4Hdr->next_proto_id == IPPROTO_TCP)
                    {
                        tcpHdr = rte_pktmbuf_mtod_offset(m, struct rte_tcp_hdr *,
                                                         sizeof(struct rte_ether_hdr) +
                                                             sizeof(struct rte_ipv4_hdr));
                        ipv45Tuple.port_src = be16toh(tcpHdr->src_port);
                        ipv45Tuple.port_dst = be16toh(tcpHdr->dst_port);
                    }
                    else if (rteIpv4Hdr->next_proto_id == IPPROTO_UDP)
                    {
                        udpHdr = rte_pktmbuf_mtod_offset(m, struct rte_udp_hdr *,
                                                         sizeof(struct rte_ether_hdr) +
                                                             sizeof(struct rte_ipv4_hdr));
                        ipv45Tuple.port_src = be16toh(udpHdr->src_port);
                        ipv45Tuple.port_dst = be16toh(udpHdr->dst_port);
                    }
                    result = rte_hash_lookup_data(dlb_d->ht_table_fm, (void *)&ipv45Tuple,
                                                  (void **)&DIP);
                    if (result == -ENOENT)
                    { // not found
                        dlb_d->unfound += 1;
                        rte_ring_enqueue(dlb_d->send_ring, &ipv45Tuple);
                        while (rte_ring_dequeue(dlb_d->recv_ring, &msg) < 0)
                        {
                            if (*(dlb_d->dp_quit))
                                goto exit;
                        }
                        int ret_add = rte_hash_add_key_data(dlb_d->ht_table_fm, &ipv45Tuple,
                                                            msg);
                        // DEBUG_PRINT("%d: get result\n", lcore_id);
                        switch (ret_add)
                        {
                        case -ENOSPC:
                            dlb_d->key_full += 1;
                            break;
                        case 0:
                            break;
                        case -EINVAL:
                            break;
                        }
                        rteIpv4Hdr->dst_addr = *(rte_be32_t *)msg;
                    }
                    else
                    {
                        rteIpv4Hdr->dst_addr = *DIP;
                        dlb_d->found += 1;
                    }
                }
                nb_tx = rte_eth_tx_buffer(portid, queue_id, txb, m);
                dlb_d->txs += nb_tx * rte_pktmbuf_pkt_len(m) * 8;
                dlb_d->txn += nb_tx;
            }
        }
    }
exit:
    // nb_rx = rte_eth_rx_burst(portid, queue_num, pkts_burst, dlb_d->max_pkt_burst);
    // for (j = 0; j < nb_rx; j++){
    //     m = pkts_burst[j];
    //     nb_tx = rte_eth_tx_buffer(portid, queue_num, txb, m);
    // }
    nb_tx = rte_eth_tx_buffer_flush(portid, queue_id, txb);
    rte_free(txb);
}

void decomposed_LB_data_create(struct decomposed_LB_data *p_dlb_d, int group_id, int data_no, struct decomposed_LB_control *p_dlb_c)
{
    char s[64];
    snprintf(s, sizeof(s), "%d_%d", group_id, data_no);
    struct rte_hash_parameters ht_table_type_fm = {
        .name = s,
        .entries = 150000, // arbitrary
        .key_len = sizeof(struct ipv4_5tuple),
        .hash_func = rte_jhash,
        .hash_func_init_val = 0,
        .socket_id = 1};

    (*p_dlb_d).ht_table_fm = rte_hash_create(&ht_table_type_fm);
    if ((*p_dlb_d).ht_table_fm == NULL)
    {
        printf("Unable to create the decomposed LB table on socket %d\n", 1);
        exit(0);
    }

    p_dlb_d->recv_ring = p_dlb_c->send_ring_plist[data_no];
    p_dlb_d->send_ring = p_dlb_c->recv_ring_plist[data_no];
}

void decomposed_LB_data_init(struct decomposed_LB_data *pp_glb_d, struct decomposed_LB_data_parameters *p_p)
{
    DEBUG_PRINT("decomposed_LB_data_init start\n");
    // *pp_glb_d = malloc(sizeof(struct decomposed_LB_data));
    (*pp_glb_d).complexity_data = p_p->complexity_data;
    (*pp_glb_d).core_id = p_p->core_id;
    (*pp_glb_d).dp_quit = p_p->dp_quit;
    (*pp_glb_d).frequency_control = p_p->frequency_control;
    (*pp_glb_d).max_pkt_burst = p_p->max_pkt_burst;
    (*pp_glb_d).queue_id = p_p->queue_id;
    (*pp_glb_d).result = 0;
    (*pp_glb_d).rxn = 0;
    (*pp_glb_d).rxs = 0;
    (*pp_glb_d).txn = 0;
    (*pp_glb_d).txs = 0;
    (*pp_glb_d).found = 0;
    (*pp_glb_d).unfound = 0;
    (*pp_glb_d).key_full = 0;

    // (*pp_glb_d).recv_ring = p_p->recv_ring;
    // (*pp_glb_d).send_ring = p_p->send_ring;

    rte_hash_reset(pp_glb_d->ht_table_fm);
    DEBUG_PRINT("decomposed_LB_data_init done\n");
}

void decomposed_LB_data_destroy(struct decomposed_LB_data *pp_glb_d)
{
    // rte_hash_free((**pp_glb_d).ht_table_fm);
    // free(*pp_glb_d);
}

int decomposed_LB_control_action(struct decomposed_LB_control *dlb_c)
{
    void *msg;
    int i = 0;
    int j = 0;
    unsigned lcore_id = rte_lcore_id();
    printf("%s entering main loop on lcore %u\n", __func__, lcore_id);
    while (!*(dlb_c->dp_quit))
    {
        // printf("%d\n", dlb_c->n_data_component);
        for (i = 0; i < dlb_c->n_data_component; i++)
        {
            // DEBUG_PRINT("%d\n", i);
            if (rte_ring_dequeue(dlb_c->recv_ring_plist[i], &msg) < 0)
            {
                continue;
            }
            else
            {
                // DEBUG_PRINT("%d\n", i);
                // printf("%d\n", i);
                dlb_c->n_control_invocation += 1;
                // operation(dlb_c->control_complexity);
                rte_ring_enqueue(dlb_c->send_ring_plist[i], (void *)&server_ip_map_array[j].ip_dst_private);
                j = (j++) % 10; // round robin
            }
            // DEBUG_PRINT("%d", i);
        }
    }
    for (int i = 0; i < 24; i++)
    {
        rte_ring_reset(dlb_c->send_ring_plist[i]);
        rte_ring_reset(dlb_c->recv_ring_plist[i]);
    }
}

void decomposed_LB_control_create(struct decomposed_LB_control *glb_c, int group_id)
{
    void *msg;
    char s[64];
    unsigned flags = 0;
    unsigned ring_size = 64;

    for (int i = 0; i < 24; i++)
    {
        snprintf(s, sizeof(s), "G %d dlb_c_control recv %d", group_id, i);
        DEBUG_PRINT("%s\n", s);
        (*glb_c).recv_ring_plist[i] = rte_ring_create(s, ring_size, rte_socket_id(), flags);
        if ((*glb_c).recv_ring_plist[i] == NULL)
            rte_exit(EXIT_FAILURE, "%s\n", rte_strerror(rte_errno));

        snprintf(s, sizeof(s), "G %d dlb_c_control send %d", group_id, i);

        (*glb_c).send_ring_plist[i] = rte_ring_create(s, ring_size, rte_socket_id(), flags);

        if ((*glb_c).send_ring_plist[i] == NULL)
            rte_exit(EXIT_FAILURE, "%s\n", rte_strerror(rte_errno));
    }
}

void decomposed_LB_control_init(struct decomposed_LB_control *glb_c, struct decomposed_LB_control_parameters *p_p)
{
    DEBUG_PRINT("decomposed_LB_control_init start\n");

    (*glb_c).n_data_component = p_p->n_data_component;
    (*glb_c).dp_quit = p_p->dp_quit;
    (*glb_c).complexity_control = p_p->complexity_control;

    DEBUG_PRINT("decomposed_LB_control_init done\n");
}

void decomposed_LB_control_destroy(struct decomposed_LB_control *glb_c)
{
    // for (int i = 0; i < (**glb_c).n_data_component; i++)
    // {
    //     rte_ring_free((**glb_c).recv_ring_plist[i]);
    //     rte_ring_free((**glb_c).send_ring_plist[i]);
    // }
    // free((*glb_c)->recv_ring_plist);
    // free((*glb_c)->send_ring_plist);
    // free(*glb_c);
}

void decomposed_LB_group_init(struct decomposed_LB_group *pp_glb_p, struct decomposed_LB_group_parameters *p_parameter_glb_m)
{
    DEBUG_PRINT("decomposed_LB_group_init start\n");
    // *pp_glb_p = malloc(sizeof(struct decomposed_LB_group));
    pp_glb_p->complexity_control = p_parameter_glb_m->complexity_control;
    pp_glb_p->complexity_data = p_parameter_glb_m->complexity_data;
    int n_data = p_parameter_glb_m->set_data_core->num;
    int n_control = 1; // only support 1 control node for now

    DEBUG_PRINT("n_data: %d\n", n_data);
    // pp_glb_p->pp_dlb_d = malloc(sizeof(struct decomposed_LB_data) * n_data);
    // pp_glb_p->pp_dlb_c = malloc(sizeof(struct decomposed_LB_control) * n_control);

    struct decomposed_LB_control_parameters control_parameter = {
        .group_id = p_parameter_glb_m->group_id,
        .complexity_control = p_parameter_glb_m->complexity_control,
        .core_id = p_parameter_glb_m->control_core, // 1 by default, needs changed later
        .dp_quit = p_parameter_glb_m->dp_quit,
        .n_data_component = n_data};

    decomposed_LB_control_init(&(pp_glb_p->dlb_c), &control_parameter);

    struct decomposed_LB_data_parameters data_parameter;
    int i;
    i = 0;
    for (struct Set_iterator *set_iterator_data_set = set_iterator_create(p_parameter_glb_m->set_data_core); set_iterator_data_set->index < 100; set_iterator_next(set_iterator_data_set))
    {
        data_parameter.group_id = p_parameter_glb_m->group_id;
        // data_parameter.recv_ring = (pp_glb_p)->dlb_c.send_ring_plist[i];
        // data_parameter.send_ring = (pp_glb_p)->dlb_c.recv_ring_plist[i];
        data_parameter.complexity_data = p_parameter_glb_m->complexity_data;
        data_parameter.core_id = set_iterator_data_set->index;
        data_parameter.dp_quit = p_parameter_glb_m->dp_quit;
        data_parameter.max_pkt_burst = p_parameter_glb_m->max_pkt_burst;
        data_parameter.queue_id = set_pop(p_parameter_glb_m->queue_set);
        ; // one group for now so it is i. Needs changed later
        data_parameter.frequency_control = p_parameter_glb_m->frequency_control;
        DEBUG_PRINT("(p_parameter_glb_m->num_entry %d", (p_parameter_glb_m->num_entry));
        data_parameter.num_entry = (p_parameter_glb_m->num_entry);
        decomposed_LB_data_init(&(pp_glb_p->dlb_d)[i], &data_parameter);
        DEBUG_PRINT("DATA_INIT %d\n", i);
        i += 1;
    }

    (*pp_glb_p).data_core = p_parameter_glb_m->set_data_core;
    DEBUG_PRINT("decomposed_LB_group_init done\n");
}

void decomposed_LB_group_create(struct decomposed_LB_group *pp_dlb_p, int group_id)
{
    decomposed_LB_control_create(&pp_dlb_p->dlb_c, group_id);
    for (int i = 0; i < 24; i++)
        decomposed_LB_data_create(&pp_dlb_p->dlb_d[i], group_id, i, &pp_dlb_p->dlb_c);
}

void decomposed_LB_group_destroy(struct decomposed_LB_group *pp_glb_p)
{
    DEBUG_PRINT("decomposed_LB_group_destroy started\n");
    // decomposed_LB_control_destroy(&((*pp_glb_p)->pp_dlb_c[0]));
    // for (int i = 0; i < (**pp_glb_p).data_core->num; i++)
    // {
    //     decomposed_LB_data_destroy(&((*pp_glb_p)->pp_dlb_d)[i]);
    // }
    // free((*pp_glb_p)->pp_dlb_c);
    // free((*pp_glb_p)->pp_dlb_d);
    // free(*pp_glb_p);
    DEBUG_PRINT("decomposed_LB_group_destroy done\n");
    return;
}
