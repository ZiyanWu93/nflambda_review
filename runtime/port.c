//
// Created by anon on 5/5/21.
//

#include "port.h"

extern struct Config *c;
static uint8_t rss_intel_key[40] = {0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x5B, 0x5A,
                                    0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x5B, 0x5A,
                                    0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x5B, 0x5A,
                                    0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x5B, 0x5A,
                                    0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x5B, 0x5A};

#define ETH_RSS_E1000_IGB (    \
    ETH_RSS_IPV4 |             \
    ETH_RSS_NONFRAG_IPV4_TCP | \
    ETH_RSS_NONFRAG_IPV4_UDP | \
    ETH_RSS_IPV6 |             \
    ETH_RSS_NONFRAG_IPV6_TCP | \
    ETH_RSS_NONFRAG_IPV6_UDP | \
    ETH_RSS_IPV6_EX |          \
    ETH_RSS_IPV6_TCP_EX |      \
    ETH_RSS_IPV6_UDP_EX)

static const struct rte_eth_conf port_conf_default = {
    .rxmode = {
        .split_hdr_size = 0,
        .mq_mode = ETH_MQ_RX_RSS,
    },
    .rx_adv_conf = {
        .rss_conf = {
            .rss_key = rss_intel_key,
            .rss_key_len = 40,
            .rss_hf = ETH_RSS_E1000_IGB,
        },
    },
    .txmode = {},
};

// memory pool
// number of cores
// port number

int port_init(struct port_parameter *portParameter)
{
//    printf("port init started\n");
    struct rte_eth_conf port_conf = port_conf_default;
    struct rte_eth_txconf txconf;
    struct rte_eth_dev_info dev_info;
    const uint16_t rx_rings = portParameter->n_queue, tx_rings = portParameter->n_queue;

    int nb_ports = rte_eth_dev_count_avail();
    if (nb_ports == 0)
        rte_exit(EXIT_FAILURE, "No Ethernet ports - bye\n");

    int retval;
    uint16_t q;

    rte_eth_dev_info_get(portParameter->port, &dev_info);
    if (dev_info.tx_offload_capa & DEV_TX_OFFLOAD_MBUF_FAST_FREE)
        port_conf.txmode.offloads |=
            DEV_TX_OFFLOAD_MBUF_FAST_FREE;

    if (!rte_eth_dev_is_valid_port(portParameter->port))
    {
        printf("error 1\n");
        return retval;
    }

    /* Configure the Ethernet device. */
    retval = rte_eth_dev_configure(portParameter->port, rx_rings, tx_rings, &port_conf);
    if (retval != 0)
    {
        printf("error 2\n");
        return retval;
    }

    retval = rte_eth_dev_adjust_nb_rx_tx_desc(portParameter->port, &(portParameter->nb_rxd), &(portParameter->nb_txd));

    if (retval != 0)
    {
        printf("error 3\n");
        return retval;
    }

    for (q = 0; q < rx_rings; q++)
    {
        retval = rte_eth_rx_queue_setup(portParameter->port, q, portParameter->nb_rxd,
                                        rte_eth_dev_socket_id(portParameter->port), NULL, portParameter->mempool);
        if (retval < 0)
        {
            printf("error 4\n");
            return retval;
        }
    }

    txconf = dev_info.default_txconf;
    txconf.offloads = port_conf.txmode.offloads;
    for (q = 0; q < tx_rings; q++)
    {
        retval = rte_eth_tx_queue_setup(portParameter->port, q, portParameter->nb_txd,
                                        rte_eth_dev_socket_id(portParameter->port), &txconf);
        if (retval < 0)
        {
            printf("error 5\n");
            return retval;
        }
    }
    //    install_flow();
    //
    /* Start the Ethernet port. */
    retval = rte_eth_dev_start(0);
    if (retval < 0)
    {
        printf("error 6\n");
        return retval;
    }

    /* Display the port MAC address. */
    struct rte_ether_addr addr;
    rte_eth_macaddr_get(portParameter->port, &addr);
    printf("Port %u MAC: %02" PRIx8
           " %02" PRIx8
           " %02" PRIx8
           " %02" PRIx8
           " %02" PRIx8
           " %02" PRIx8
           "\n",
           portParameter->port,
           addr.addr_bytes[0], addr.addr_bytes[1],
           addr.addr_bytes[2], addr.addr_bytes[3],
           addr.addr_bytes[4], addr.addr_bytes[5]);
    /* Enable RX in promiscuous mode for the Ethernet device. */
    rte_eth_promiscuous_enable(portParameter->port);
//    printf("port init done\n");
    return 1;
}
