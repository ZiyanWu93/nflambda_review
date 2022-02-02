//
// Created by anon on 9/15/2021.
//

#ifndef NF_PLATFORM_TRAFFIC_GENERATOR_H
#define NF_PLATFORM_TRAFFIC_GENERATOR_H

#include "stdint.h"
#include "../common.h"
#include "rte_byteorder.h"
#include "rte_udp.h"
#include "rte_ip.h"
#include "rte_ether.h"


#define TX_PACKET_LENGTH 862
#define IP_DEFTTL  64   /* from RFC 1340. */
#define UDP_SRC_PORT 6666
#define UDP_DST_PORT 6666
#define IP_VERSION 0x40
#define IP_HDRLEN  0x05 /* default IP header length == five 32-bits words. */
#define IP_VHL_DEF (IP_VERSION | IP_HDRLEN)
uint32_t IP_SRC_ADDR, IP_DST_ADDR;

#if RTE_BYTE_ORDER == RTE_BIG_ENDIAN
#define RTE_BE_TO_CPU_16(be_16_v)  (be_16_v)
#define RTE_CPU_TO_BE_16(cpu_16_v) (cpu_16_v)
#else
#define RTE_BE_TO_CPU_16(be_16_v) \
(uint16_t) ((((be_16_v) & 0xFF) << 8) | ((be_16_v) >> 8))
#define RTE_CPU_TO_BE_16(cpu_16_v) \
(uint16_t) ((((cpu_16_v) & 0xFF) << 8) | ((cpu_16_v) >> 8))
#endif

static struct rte_ipv4_hdr pkt_ip_hdr;  /**< IP header of transmitted packets. */
static struct rte_udp_hdr pkt_udp_hdr; /**< UDP header of transmitted packets. */
struct rte_ether_addr my_addr; // SRC MAC address of NIC

struct traffic_gen_data {

};
struct traffic_gen_control_parameters{
    struct rte_mempool *mempool;
};
struct traffic_gen_control {
    struct rte_mempool *mempool
};

struct traffic_gen_control * traffic_gen_control_init(struct traffic_gen_control_parameters tgc_p);
// helper function
static void setup_pkt_udp_ip_headers(struct rte_ipv4_hdr *ip_hdr,
                                     struct rte_udp_hdr *udp_hdr,
                                     uint16_t pkt_data_len) {
    uint16_t *ptr16;
    uint32_t ip_cksum;
    uint16_t pkt_len;

    //Initialize UDP header.
    pkt_len = (uint16_t) (pkt_data_len + sizeof(struct rte_udp_hdr));
    udp_hdr->src_port = rte_cpu_to_be_16(UDP_SRC_PORT);
    udp_hdr->dst_port = rte_cpu_to_be_16(UDP_DST_PORT);
    udp_hdr->dgram_len = RTE_CPU_TO_BE_16(pkt_len);
    udp_hdr->dgram_cksum = 0; /* No UDP checksum. */

    //Initialize IP header.
    pkt_len = (uint16_t) (pkt_len + sizeof(struct rte_ipv4_hdr));
    ip_hdr->version_ihl = IP_VHL_DEF;
    ip_hdr->type_of_service = 0;
    ip_hdr->fragment_offset = 0;
    ip_hdr->time_to_live = IP_DEFTTL;
    ip_hdr->next_proto_id = IPPROTO_UDP;
    ip_hdr->packet_id = 0;
    ip_hdr->total_length = RTE_CPU_TO_BE_16(pkt_len);
    ip_hdr->src_addr = rte_cpu_to_be_32(IP_SRC_ADDR);
    ip_hdr->dst_addr = rte_cpu_to_be_32(IP_DST_ADDR);

    //Compute IP header checksum.
    ptr16 = (unaligned_uint16_t *) ip_hdr;
    ip_cksum = 0;
    ip_cksum += ptr16[0];
    ip_cksum += ptr16[1];
    ip_cksum += ptr16[2];
    ip_cksum += ptr16[3];
    ip_cksum += ptr16[4];
    ip_cksum += ptr16[6];
    ip_cksum += ptr16[7];
    ip_cksum += ptr16[8];
    ip_cksum += ptr16[9];

    //Reduce 32 bit checksum to 16 bits and complement it.
    ip_cksum = ((ip_cksum & 0xFFFF0000) >> 16) +
               (ip_cksum & 0x0000FFFF);
    if (ip_cksum > 65535)
        ip_cksum -= 65535;
    ip_cksum = (~ip_cksum) & 0x0000FFFF;
    if (ip_cksum == 0)
        ip_cksum = 0xFFFF;
    ip_hdr->hdr_checksum = (uint16_t) ip_cksum;
}

//
//// actually send the packet
static struct rte_mbuf *send_packet(struct traffic_gen_control *tg_c) {
    struct rte_mbuf *pkt;
    union {
        uint64_t as_int;
        struct rte_ether_addr as_addr;
    } dst_eth_addr;
    struct rte_ether_hdr eth_hdr;
//    struct rte_mbuf *pkts_burst[1];

    pkt = rte_mbuf_raw_alloc(tg_c->mempool);
    if (pkt == NULL) {
        printf("trouble at rte_mbuf_raw_alloc\n");
        exit(1);
    }
    rte_pktmbuf_reset_headroom(pkt);
    pkt->data_len = TX_PACKET_LENGTH;

    // set up addresses
    dst_eth_addr.as_int = rte_cpu_to_be_64(0ULL);
    rte_ether_addr_copy(&dst_eth_addr.as_addr, &eth_hdr.d_addr);
    rte_ether_addr_copy(&my_addr, &eth_hdr.s_addr);
    eth_hdr.ether_type = rte_cpu_to_be_16(RTE_ETHER_TYPE_IPV4);

    // copy header to packet in mbuf
    rte_memcpy(rte_pktmbuf_mtod_offset(pkt, char *, 0),
               &eth_hdr, (size_t) sizeof(eth_hdr));
    rte_memcpy(rte_pktmbuf_mtod_offset(pkt, char *, sizeof(struct rte_ether_hdr)),
               &pkt_ip_hdr, (size_t) sizeof(pkt_ip_hdr));
    rte_memcpy(rte_pktmbuf_mtod_offset(pkt, char *,
                                       sizeof(struct rte_ether_hdr) + sizeof(struct rte_ipv4_hdr)),
               &pkt_udp_hdr, (size_t) sizeof(pkt_udp_hdr));

    // Add some pkt fields
    pkt->nb_segs = 1;
    pkt->pkt_len = pkt->data_len;
    pkt->ol_flags = 0;
    // Actually send the packet
//    pkts_burst[0] = pkt;
//    const uint16_t nb_tx = rte_eth_tx_burst(0, 0, pkts_burst, 1);
//    packet_count += nb_tx;
//    rte_mbuf_raw_free(pkt);
    return pkt;
}

//
//static inline void traffic_gen_data_action(struct traffic_gen_data *gdd_state, struct message *m) {
//    switch (m->event) {
//        case CONTROL_TO_DATA:
////            DEBUG_PRINT("%s" , m->data);
//            m->event = TRAN_MALLOC_FREE_MESSAGE;
//            return;
//    }
//    m->event = DROP_PACKET;
//};
//
//static inline void traffic_gen_control_action(struct traffic_gen_control *genControl, struct message *m) {
//    switch (m->event) {
//        case CUSTOM_EVENT1:
//            // prepare packets
//            m->m = send_packet(genControl);
//            m->event = SEND_PACKET;
//            break;
//        case CUSTOM_EVENT2:
//            break;
//        case COORDINATOR_TO_CONTROL:
//            DEBUG_PRINT("%s" , m->data);
//            m->event = CONTROL_TO_DATA;
//            m->m = send_packet(genControl);
////            m->data = "hello world from the controller\n";
//            break;
//    }
//};


#endif //NF_PLATFORM_TRAFFIC_GENERATOR_H
