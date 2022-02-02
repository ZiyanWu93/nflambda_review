//
// Created by anon on 4/24/21.
//

#include <rte_hash.h>
#include "mono_lb.h"
#include "rte_jhash.h"
#include "../Config.h"
#include "../common/defined.h"

static void DumpHex(const void *data, size_t size)
{
    char ascii[17];
    size_t i, j;
    ascii[16] = '\0';
    for (i = 0; i < size; ++i)
    {
        printf("%02X ", ((const unsigned char *)data)[i]);
        if (((const unsigned char *)data)[i] >= ' ' && ((const unsigned char *)data)[i] <= '~')
        {
            ascii[i % 16] = ((const unsigned char *)data)[i];
        }
        else
        {
            ascii[i % 16] = '.';
        }
        if ((i + 1) % 8 == 0 || i + 1 == size)
        {
            printf(" ");
            if ((i + 1) % 16 == 0)
            {
                printf("|  %s \n", ascii);
            }
            else if (i + 1 == size)
            {
                ascii[(i + 1) % 16] = '\0';
                if ((i + 1) % 16 <= 8)
                {
                    printf(" ");
                }
                for (j = (i + 1) % 16; j < 16; ++j)
                {
                    printf("   ");
                }
                printf("|  %s \n", ascii);
            }
        }
    }
}

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

int mono_lb_action(struct mono_lb *monolb)
{
    // get 5-tuples
    // lookup using flow mapper hashing table
    // if miss, call control action
    // if hit, rewrite the destination ip and forward it

    uint32_t *DIP;
    struct rte_ether_hdr *rteEtherHdr;
    struct rte_ipv4_hdr *rteIpv4Hdr;
    struct rte_tcp_hdr *tcpHdr;
    struct ipv4_5tuple ipv45Tuple;

    struct rte_mbuf *pkts_burst[monolb->max_pkt_burst * 2];
    int ret;
    unsigned lcore_id = rte_lcore_id();
    unsigned j, portid, nb_tx;
    unsigned nb_rx = 0;
    bool started = false;
    int queue_id = monolb->queue_id;
    struct rte_mbuf *m;
    const uint64_t drain_tsc = (rte_get_tsc_hz() + US_PER_S - 1) / US_PER_S *
                               BURST_TX_DRAIN_US;

    uint64_t cur_tsc, diff_tsc, prev_tsc;
    prev_tsc = 0;

    struct rte_eth_dev_tx_buffer *txb = rte_zmalloc_socket("tx_buffer",
                                                           RTE_ETH_TX_BUFFER_SIZE(monolb->max_pkt_burst) * 2, 0,
                                                           1);
    if (txb == NULL)
        rte_exit(EXIT_FAILURE, "Cannot allocate buffer for tx on port %u\n",
                 portid);

    rte_eth_tx_buffer_init(txb, monolb->max_pkt_burst);

    if (ret < 0)
        rte_exit(EXIT_FAILURE,
                 "Cannot set error callback for tx buffer on port %u\n",
                 portid);

    printf("mono LB entering main loop on lcore %u\n", lcore_id);

    // monolb->last_port = 0;
    int packet_size = 0;
    int result;

    while (!*(monolb->dp_quit))
    {
        cur_tsc = rte_rdtsc();
        diff_tsc = cur_tsc - prev_tsc;

        if (unlikely(diff_tsc > drain_tsc))
        {
            nb_tx = rte_eth_tx_buffer_flush(portid, queue_id, txb);
            monolb->txn += nb_tx;
            monolb->txs += nb_tx * packet_size * 8;
            prev_tsc = cur_tsc;
        }
        nb_rx = rte_eth_rx_burst(portid, queue_id, pkts_burst, monolb->max_pkt_burst);
        monolb->rxn += nb_rx;
        if (unlikely(nb_rx == 0))
            continue;

        if (nb_rx != 0)
        {
            if (unlikely(started == false))
                monolb->start = rte_rdtsc();

            started = true;
            // use rteIpv4Hdr as pointer will lead to segmentation fault // solution: the checking of different headers seems to be necessary
            for (j = 0; j < nb_rx; j++)
            {
                //                rte_prefetch0(rte_pktmbuf_mtod(m,void *)); this will lead to segmentation fault?
                m = pkts_burst[j];
                packet_size = rte_pktmbuf_pkt_len(m);
                monolb->rxs += packet_size * 8;
                rteEtherHdr = rte_pktmbuf_mtod(m, struct rte_ether_hdr *);

                if (rteEtherHdr->ether_type == rte_be_to_cpu_16(RTE_ETHER_TYPE_IPV4))
                {
                    rteIpv4Hdr = rte_pktmbuf_mtod_offset(m, struct rte_ipv4_hdr *,
                                                         sizeof(struct rte_ether_hdr));
                    if (rteIpv4Hdr->next_proto_id == IPPROTO_TCP)
                    {
                        tcpHdr = rte_pktmbuf_mtod_offset(m, struct rte_tcp_hdr *,
                                                         sizeof(struct rte_ether_hdr) +
                                                             sizeof(struct rte_ipv4_hdr));

                        // action
                        ipv45Tuple.ip_src = rte_be_to_cpu_32((rteIpv4Hdr->src_addr));
                        ipv45Tuple.ip_dst = rte_be_to_cpu_32((rteIpv4Hdr->dst_addr));
                        ipv45Tuple.proto = (uint8_t)rteIpv4Hdr->next_proto_id;
                        ipv45Tuple.port_src = be16toh(tcpHdr->src_port);
                        ipv45Tuple.port_dst = be16toh(tcpHdr->dst_port);
                        result = rte_hash_lookup_data(monolb->ht_table_fm, (void *)&ipv45Tuple,
                                                      (void **)&DIP);
                        if (result == -ENOENT)
                        { // not found
                            monolb->unfound += 1;
                            int ret_add = rte_hash_add_key_data(monolb->ht_table_fm, &ipv45Tuple,
                                                                (void *)&server_ip_map_array[0].ip_dst_private);
                            switch (ret_add)
                            {
                            case -ENOSPC:
                                monolb->key_full += 1;
                                break;
                            case 0:
                                break;
                            case -EINVAL:
                                break;
                            }
                        }
                        else
                        {
                            rteIpv4Hdr->dst_addr = *DIP;
                            monolb->found += 1;
                        }
                    }
                }
                nb_tx = rte_eth_tx_buffer(portid, queue_id, txb, m);
                monolb->txs += nb_tx * rte_pktmbuf_pkt_len(m) * 8;
                monolb->txn += nb_tx;
            }
        }
    }
}

void mono_lb_create(struct mono_lb *p_lb, int id)
{
    char s[64];
    /* create hash */
    snprintf(s, sizeof(s), "hash %d", id);

    struct rte_hash_parameters ht_table_type_fm = {
        .name = s,
        .entries = 150000, // arbitrary
                           //                .entries = 1000000, // arbitrary
                           //                .entries = 2000000, // arbitrary
                           //                .entries = 20000000, // arbitrary
        .key_len = sizeof(struct ipv4_5tuple),
        .hash_func = rte_jhash,
        .hash_func_init_val = 0,
        //.extra_flag = (unsigned) mono_lb_list[i].opts->data.flow_mapper.extra_flags.value,
        .socket_id = 1};
    (*p_lb).ht_table_fm = rte_hash_create(&ht_table_type_fm);
    if ((*p_lb).ht_table_fm == NULL)
    {
        printf("Unable to create the monolb_fm table on socket %d\n", 1);
    }
}

void mono_lb_init(struct mono_lb *p_lb, int queue_id, int lcore_id, int max_pkt_burst, volatile bool *dp_quit)
{
    (*p_lb).rxn = 0;
    (*p_lb).txn = 0;
    (*p_lb).rxs = 0;
    (*p_lb).txs = 0;
    (*p_lb).key_full = 0;
    (*p_lb).dp_quit = dp_quit;
    (*p_lb).max_pkt_burst = max_pkt_burst;
    (*p_lb).queue_id = queue_id;
    (*p_lb).found = 0;
    (*p_lb).unfound = 0;
}

void mono_lb_destroy(struct mono_lb **p_lb)
{
    // free the hash_table
    rte_hash_free((**p_lb).ht_table_fm);
    free(*p_lb);
    *p_lb = NULL;
    return;
}