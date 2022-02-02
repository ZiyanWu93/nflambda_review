#include "Config.h"
#include "coordinator.h"
#include <libltdl/lt_system.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

struct config;
extern struct Config *c;

void (*action_list[256])(void *, struct message *) ={
        fwder_action,
        generic_decomposed_data_action,
        generic_decomposed_control_action
};

void install_flow() {
    struct rte_flow_error error;
    struct rte_flow_attr attr;
    struct rte_flow_item pattern[4];
    struct rte_flow_action action[2];
    struct rte_flow *flow;
    struct rte_flow_action_queue queue;
    struct rte_flow_item_ipv4 ip_spec;
    struct rte_flow_item_ipv4 ip_mask;
    struct rte_flow_item_tcp tcp_spec;
    struct rte_flow_item_tcp tcp_mask;
    uint32_t src_ip;
    uint32_t dst_ip;
    for (int i = 0; i < 24; i++) {
        uint16_t queue_id = i;
        queue.index = queue_id;

        printf("install flow rule\n");

        src_ip = (((0) << 24) + (0 << 16) + (0 << 8) + 0); /* src ip = 0.0.0.0 */
        dst_ip = ((0 << 24) + (0 << 16) + (0 << 8) + 0);
        int res;

        flow = NULL;
        memset(pattern, 0, sizeof(pattern));
        memset(action, 0, sizeof(action));

        memset(&attr, 0, sizeof(struct rte_flow_attr));
        attr.ingress = 1;

        action[0].type = RTE_FLOW_ACTION_TYPE_QUEUE;
        action[0].conf = &queue;
        action[1].type = RTE_FLOW_ACTION_TYPE_END;

        pattern[0].type = RTE_FLOW_ITEM_TYPE_ETH;

        //        memset(&ip_spec, 0, sizeof(struct rte_flow_item_ipv4));
        //        memset(&ip_mask, 0, sizeof(struct rte_flow_item_ipv4));
        //        ip_spec.hdr.dst_addr = htonl(dst_ip);
        //        ip_mask.hdr.dst_addr = EMPTY_MASK;
        //        ip_spec.hdr.src_addr = htonl(src_ip);
        //        ip_mask.hdr.src_addr = EMPTY_MASK;

        pattern[1].type = RTE_FLOW_ITEM_TYPE_IPV4;
        //        pattern[1].spec = &ip_spec;
        //        pattern[1].mask = &ip_mask;

        memset(&tcp_spec, 0, sizeof(struct rte_flow_item_tcp));
        memset(&tcp_mask, 0, sizeof(struct rte_flow_item_tcp));
        tcp_spec.hdr.dst_port = RTE_BE16(i);
        tcp_mask.hdr.dst_port = 0xffff;
        tcp_spec.hdr.src_port = RTE_BE16(0);
        tcp_mask.hdr.src_port = 0x0;

        pattern[2].type = RTE_FLOW_ITEM_TYPE_TCP;
        pattern[2].spec = &tcp_spec;
        pattern[2].mask = &tcp_mask;

        /* the final level must be always type end */
        pattern[3].type = RTE_FLOW_ITEM_TYPE_END;

        res = rte_flow_validate(0, &attr, pattern, action, &error);
        if (!res) {
            flow = rte_flow_create(0, &attr, pattern, action, &error);
        }

        if (!flow) {
            printf("Flow can't be created %d message: %s\n",
                   error.type,
                   error.message ? error.message : "(no stated reason)");
            rte_exit(EXIT_FAILURE, "error in creating flow");
        }
    }
}


void init_EAL(char *filename) {
    FILE *fp;
    char *line = NULL;
    int length = 0;
    size_t len = 0;
    size_t read;

    int argc;
    char **argv;
    fp = fopen(filename, "r");

    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
        length += 1;
    }
    fclose(fp);

    printf("the length of the file is %d\n", length);

    fp = fopen(filename, "r");
    len = 0;
    argc = length;
    argv = malloc(sizeof(char *) * argc);

    for (int i = 0; i < argc; i++) {
        (argv)[i] = malloc(sizeof(char) * 100);
    }
    fclose(fp);

    fp = fopen(filename, "r");
    len = 0;
    if (fp == NULL)
        exit(EXIT_FAILURE);

    int i = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
        int len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
            line[len - 1] = '\0';

        strcpy((argv)[i], line);
        i += 1;
    }
    fclose(fp);

    printf("the running command is: ");
    for (int i = 0; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");

    // initialize EAL
    int ret = rte_eal_init(argc, argv);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Invali  d EAL arguments\n");

    uint16_t lcore_id;
    i = 0;

    int nb_ports = rte_eth_dev_count_avail();
    argc -= ret;
    argv += ret;
}

void config_init(char *config_file) {
    c = malloc(sizeof(struct Config));

    c->hz = rte_get_timer_hz();
    printf("The number of cycles in one second is %lu\n", c->hz);


    c->portParameter = malloc(sizeof(struct port_parameter));

    c->nb_txd = 2048; // descriptors per queue
    c->nb_rxd = 1024; // descriptors per queue

    c->nf = NF_FWD;
    c->num_mbufs = 131071;
    c->max_pkt_burst = 1024;
    c->mbuf_cache_size = 256;

    c->stateTable.num = 0;
    c->actionTable.num = 0;

    struct port_parameter portParameter = {
            .nb_rxd = c->nb_rxd,
            .nb_txd = c->nb_txd,
            .port = 0,
            .n_queue = 0,
            .num_mbufs = c->num_mbufs,
            .mbuf_cache_size = c->mbuf_cache_size};

    *(c->portParameter) = portParameter;

    c->cp_quit = false;
    c->dp_quit = false;

    c->ring_size = 1024;
    c->log_started = false;

    c->portParameter->mempool = rte_pktmbuf_pool_create("MBUF_POOL", c->portParameter->num_mbufs,
                                                        c->portParameter->mbuf_cache_size, 0,
                                                        RTE_MBUF_DEFAULT_BUF_SIZE,
                                                        rte_eth_dev_socket_id(0));


    c->pm = create_physical_machine(NULL);

    register_action(&(c->actionRegistration[0]), NULL, "packet_in");
    register_action(&(c->actionRegistration[1]), NULL, "packet_out");
    register_action(&(c->actionRegistration[2]), NULL, "drop");
    register_action(&(c->actionRegistration[3]), generic_decomposed_classifier, "gd_classifier");
    register_action(&(c->actionRegistration[4]), generic_decomposed_data_action,
                    "gd_data");
    register_action(&(c->actionRegistration[5]), generic_decomposed_control_action,
                    "gd_control");

//    register_state(&(c->stateRegistration[0]), generic_decomposed_data_init, "gdd_state");
//    register_state(&(c->stateRegistration[1]), generic_decomposed_control_init, "gdc_state");


    for (int i = 0; i < 48; i++) {
        c->worker_list[i].core_id = i;
        c->worker_list[i].port_id = 0;
        c->worker_list[i].max_pkt_burst = c->max_pkt_burst;
        c->worker_list[i].dp_quit = &(c->dp_quit);
        c->worker_list[i].queue_id = i / 2; // each worker on the socket 1 will have a queue_id
        c->worker_list[i].self_mailbox = create_mailbox(i, c->ring_size);
        c->worker_list[i].message_buffer = create_zbuffer(sizeof(struct message), 131071);
        c->worker_list[i].actionTable = &c->actionTable;
        c->worker_list[i].last_action = false;
        for (int j = 0; j < MAXIMUM_PATH; j++) {
            c->worker_list[i].first_action_id[j] = -1;
        }
        c->worker_list[i].first_action_num = 0;
        c->worker_list[i].starting_bucket = -1;
        c->worker_list[i].suspend = true;
        c->worker_list[i].bucket_to_action_id = c->bucket_to_action_id;
        c->worker_list[i].operations_n = 0;
    }

    for (int i = 0; i < 48; i++) {
        for (int j = 0; j < 48; j++) {
            c->worker_list[i].other_mailbox[j] = (c->worker_list[j].self_mailbox);
        }
    }

    c->queue_num = 0;

    for (int i = 0; i < MAXIMUM_PATH; i++) {
        c->initStateParameter.path_instance_number[i] = 0;
        for (int j = 0; j < MAXIMUM_PATH_INSTANCE; j++) {
            c->initStateParameter.path[i][j] = -1;
            c->initStateParameter.rand_range = 100;
        }
    }

    for (int i = 0; i < 100; i++) {
        c->initStateParameter.execution_id = i;
        c->stateTable.stateTableEntry[i].state = state_init(& c->initStateParameter);
        c->stateTable.num++;
    }


    rte_eth_dev_info_get(0, &c->info);



    c->running = false;

    memset(c->reta_conf, 0, sizeof(c->reta_conf));
    for (int j = 0; j < RSS_I_TABLE_SIZE / RTE_RETA_GROUP_SIZE;
         j++)
        c->reta_conf[j].mask = ~0LL;
}

static inline int exists(const char *fname) {
    FILE *file;
    if ((file = fopen(fname, "r"))) {
        fclose(file);
        return 1;
    }
    return 0;
}


void output_log(struct Config *c) {
    uint64_t cur_tsc = rte_rdtsc();
    uint64_t diff_tsc = cur_tsc - c->p_time;
    const uint64_t drain_tsc = (rte_get_tsc_hz() + US_PER_S - 1) / US_PER_S *
                               LOG_INTERVAL_US;
    FILE *fptr;

    struct timeval tv;
    time_t nowtime;
    struct tm *nowtm;
    char tmbuf[64], buf[64];
    int worker_id;
    bool flag = false;
    if ((diff_tsc > drain_tsc)) {
        gettimeofday(&tv, NULL);
        nowtime = tv.tv_sec;
        nowtm = localtime(&nowtime);
        strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", nowtm);
        snprintf(buf, sizeof buf, "%s.%02ld", tmbuf, tv.tv_usec);
        // output log file
        // open a file
        // loop over the number of messages
        if (c->log_started == false) {
            fptr = fopen("/tmp/nf_platform_log.csv", "w");
            c->log_started = true;
        } else {
            fptr = fopen("/tmp/nf_platform_log.csv", "a");
        }
        if (fptr == NULL) {
            printf("Could not open file\n");
            return;
        }
        // output worker statistic
        for (int i = 0; i < c->pm->socket_num; i++) {
            for (int j = 0; j < SOCKET_CORE_NUM; j++) {
                flag = true;
                worker_id = c->pm->socketNodes[i].cores[j].lcore_id;
                fprintf(fptr, "%s,worker,%d,%ld,%ld\n", buf, worker_id, c->worker_list[worker_id].rxn,c->worker_list[worker_id].operations_n);
            }
        }
//        if (flag == true) {
//            for (int i = 0; i < c->stateTable.num; i++) {
//                fprintf(fptr, "%s,state,%d,%d\n", buf, i, c->stateTable.stateTableEntry[i].state->n_message);
//            }
//        }
        fclose(fptr);
        c->p_time = cur_tsc;
        return;
    } else {
        return;
    }
}